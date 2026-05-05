/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file module_loader.c
 * @brief Module graph orchestration: loading, dependency resolution, and source stitching.
 */

#include "module-loading/module_loader.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/common.h"
#include <module-loading/internal/module_parser.h>
#include <module-loading/internal/module_path.h>
#include "clx_os/fs.h"
#include "clx_os/path.h"
#include "utils/string_utils.h"

typedef struct module_record_t {
    char * canonicalPath;
    export_list_t exports;
    bool isLoading;
    bool isEmitted;
    struct module_record_t * next;
} module_record_t;

typedef struct {
    module_record_t * modules;
    char ** loadingStack;
    size_t loadingStackCount;
    size_t loadingStackCapacity;
    source_buffer_t stitchedSource;
    bool hadError;
} module_context_t;

static void module_loader_cleanup_context(module_context_t *);
static module_record_t * module_loader_find_module(module_context_t *, char const *);
static void module_loader_free_module(module_record_t *);
static module_record_t * module_loader_process_module(module_context_t *, char const *);
static module_record_t * module_loader_process_module_from(module_context_t *, char const *, char const *);
static bool module_loader_validate_named_imports(module_context_t *, module_record_t *, import_spec_t const *,
                                                 char const *);
static void module_loader_error(module_context_t *, char const *, ...);

/// Runtime-overridable stdlib search path.  NULL means use the discovery chain.
static char * module_loader_stdlib_path_override = NULL;

/// Cached exe-relative stdlib path. Computed once; NULL means not yet tried.
static char * module_loader_exe_relative_stdlib_path = NULL;
/// True once we have attempted exe-relative resolution (even if it failed).
static bool module_loader_exe_relative_resolved = false;

void module_loader_set_stdlib_path(char const * path) {
    if (!path) {
        free(module_loader_stdlib_path_override);
        module_loader_stdlib_path_override = NULL;
        return;
    }

    char * copiedPath = string_utils_strdup(path);
    if (!copiedPath) {
        return;
    }

    free(module_loader_stdlib_path_override);
    module_loader_stdlib_path_override = copiedPath;
}

/// Returns the effective stdlib directory using a C-compiler-style resolution chain.
/// Precedence (highest to lowest):
///   1. Explicit runtime override via module_loader_set_stdlib_path()
///   2. CELLOX_STDLIB_DIR environment variable
///   3. Executable-relative:  <exedir>/../lib/cellox/stdlib
///   4. Compile-time baked-in path CLX_STDLIB_PATH (last resort / dev fallback)
static char const * module_loader_get_stdlib_path(void) {
    if (module_loader_stdlib_path_override) {
        return module_loader_stdlib_path_override;
    }

    char const * env = getenv("CELLOX_STDLIB_DIR");
    if (env && env[0]) {
        return env;
    }

    if (!module_loader_exe_relative_resolved) {
        module_loader_exe_relative_resolved = true;
        char * exeDir = clx_os_path_executable_dir();
        if (exeDir) {
            char * candidate = module_path_join(exeDir, "../lib/cellox/stdlib");
            free(exeDir);
            if (candidate && clx_os_fs_path_exists(candidate)) {
                module_loader_exe_relative_stdlib_path = candidate;
            } else {
                free(candidate);
            }
        }
    }
    if (module_loader_exe_relative_stdlib_path) {
        return module_loader_exe_relative_stdlib_path;
    }

#ifdef CLX_STDLIB_PATH
    return CLX_STDLIB_PATH;
#else
    return NULL;
#endif
}

char * module_loader_load_program(char const * entryPath) {
    module_context_t context;
    context.modules = NULL;
    context.loadingStack = NULL;
    context.loadingStackCount = 0;
    context.loadingStackCapacity = 0;
    context.stitchedSource.chars = NULL;
    context.stitchedSource.length = 0;
    context.stitchedSource.capacity = 0;
    context.hadError = false;

    module_record_t * entryModule = module_loader_process_module(&context, entryPath);
    if (!entryModule || context.hadError) {
        module_loader_cleanup_context(&context);
        return NULL;
    }

    module_parser_append_source(&context.stitchedSource, "\0", 1);
    char * result = context.stitchedSource.chars;
    context.stitchedSource.chars = NULL;
    context.stitchedSource.length = 0;
    context.stitchedSource.capacity = 0;

    module_loader_cleanup_context(&context);
    return result;
}

static void module_loader_cleanup_context(module_context_t * context) {
    free(context->loadingStack);
    context->loadingStack = NULL;
    context->loadingStackCount = 0;
    context->loadingStackCapacity = 0;

    module_record_t * module = context->modules;
    while (module) {
        module_record_t * next = module->next;
        module_loader_free_module(module);
        module = next;
    }
    context->modules = NULL;

    free(context->stitchedSource.chars);
    context->stitchedSource.chars = NULL;
    context->stitchedSource.length = 0;
    context->stitchedSource.capacity = 0;
}

static module_record_t * module_loader_find_module(module_context_t * context, char const * canonicalPath) {
    for (module_record_t * module = context->modules; module; module = module->next) {
        if (!strcmp(module->canonicalPath, canonicalPath)) {
            return module;
        }
    }
    return NULL;
}

static void module_loader_free_module(module_record_t * module) {
    free(module->canonicalPath);
    module_parser_export_list_free(&module->exports);
    free(module);
}

static module_record_t * module_loader_process_module(module_context_t * context, char const * rawPath) {
    return module_loader_process_module_from(context, rawPath, NULL);
}

static module_record_t * module_loader_process_module_from(module_context_t * context, char const * rawPath,
                                                           char const * importerPath) {
    char * canonicalPath = module_path_canonicalize(rawPath);
    if (!canonicalPath) {
        if (importerPath) {
            module_loader_error(context, "Could not resolve module \"%s\" imported from \"%s\"", rawPath,
                                importerPath);
        } else {
            module_loader_error(context, "Could not resolve module \"%s\"", rawPath);
        }
        return NULL;
    }

    module_record_t * module = module_loader_find_module(context, canonicalPath);
    if (!module) {
        module = calloc(1, sizeof(module_record_t));
        if (!module) {
            free(canonicalPath);
            module_loader_error(context, "Not enough memory while creating module record");
            return NULL;
        }
        module->canonicalPath = canonicalPath;
        module->next = context->modules;
        context->modules = module;
    } else {
        free(canonicalPath);
    }

    if (module->isEmitted) {
        return module;
    }

    if (module->isLoading) {
        module_loader_error(context, "Cyclic module import detected for \"%s\"", module->canonicalPath);
        return NULL;
    }

    module->isLoading = true;

    if (context->loadingStackCount + 1 > context->loadingStackCapacity) {
        size_t newCapacity = context->loadingStackCapacity ? context->loadingStackCapacity * 2 : 8;
        char ** grownStack = realloc(context->loadingStack, newCapacity * sizeof(char *));
        if (!grownStack) {
            module_loader_error(context, "Not enough memory while tracking module loading stack");
            module->isLoading = false;
            return NULL;
        }
        context->loadingStack = grownStack;
        context->loadingStackCapacity = newCapacity;
    }
    context->loadingStack[context->loadingStackCount++] = module->canonicalPath;

    char * source = string_utils_read_file(module->canonicalPath, NULL, NULL);
    if (!source) {
        module_loader_error(context, "Could not read module \"%s\"", module->canonicalPath);
        module->isLoading = false;
        context->loadingStackCount--;
        return NULL;
    }

    import_spec_t * imports = NULL;
    size_t importCount = 0;
    char * transformedSource = NULL;
    export_list_t exports = {NULL, 0, 0};
    if (!module_parser_parse(source, &exports, &imports, &importCount, &transformedSource)) {
        free(source);
        module_loader_error(context, "Invalid import or export declaration in \"%s\"", module->canonicalPath);
        module_parser_export_list_free(&exports);
        module_parser_cleanup_imports(imports, importCount);
        module->isLoading = false;
        context->loadingStackCount--;
        return NULL;
    }
    free(source);
    module->exports = exports;

    for (size_t i = 0; i < importCount; i++) {
        char * resolvedImportPath = module_path_resolve_import(module->canonicalPath, imports[i].path,
                                                                module_loader_get_stdlib_path());
        if (!resolvedImportPath) {
            module_loader_error(context, "Could not resolve import path \"%s\" from \"%s\"", imports[i].path,
                                module->canonicalPath);
            module_parser_cleanup_imports(imports, importCount);
            free(transformedSource);
            module->isLoading = false;
            context->loadingStackCount--;
            return NULL;
        }

        module_record_t * importedModule =
            module_loader_process_module_from(context, resolvedImportPath, module->canonicalPath);
        free(resolvedImportPath);
        if (!importedModule) {
            module_parser_cleanup_imports(imports, importCount);
            free(transformedSource);
            module->isLoading = false;
            context->loadingStackCount--;
            return NULL;
        }

        if (!module_loader_validate_named_imports(context, importedModule, &imports[i], module->canonicalPath)) {
            module_parser_cleanup_imports(imports, importCount);
            free(transformedSource);
            module->isLoading = false;
            context->loadingStackCount--;
            return NULL;
        }
    }

    module_parser_cleanup_imports(imports, importCount);
    module_parser_append_source(&context->stitchedSource, transformedSource, strlen(transformedSource));
    free(transformedSource);

    module->isLoading = false;
    module->isEmitted = true;
    context->loadingStackCount--;
    return module;
}

static bool module_loader_validate_named_imports(module_context_t * context, module_record_t * module,
                                                 import_spec_t const * importSpec, char const * importerPath) {
    for (size_t i = 0; i < importSpec->importedNameCount; i++) {
        bool found = false;
        for (size_t j = 0; j < module->exports.count; j++) {
            if (!strcmp(importSpec->importedNames[i], module->exports.names[j])) {
                found = true;
                break;
            }
        }

        if (!found) {
            module_loader_error(context, "Unknown export \"%s\" in \"%s\" imported from \"%s\"",
                                importSpec->importedNames[i], module->canonicalPath, importerPath);
            return false;
        }
    }

    return true;
}

static void module_loader_error(module_context_t * context, char const * format, ...) {
    if (context) {
        context->hadError = true;
    }

    if (!format || !format[0]) {
        return;
    }

    va_list args;
    va_start(args, format);
    fprintf(stderr, "Module error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

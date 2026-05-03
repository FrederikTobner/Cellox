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
 * @brief File containing module loading and source stitching logic.
 */

#include "module_loader.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define PATH_SEPARATOR '\\'
#else
#include <limits.h>
#include <unistd.h>
#define PATH_SEPARATOR '/'
#endif

#include "common.h"

typedef struct {
    char * path;
    char ** importedNames;
    size_t importedNameCount;
} import_spec_t;

typedef struct {
    char ** names;
    size_t count;
    size_t capacity;
} export_list_t;

typedef struct module_record_t {
    char * canonicalPath;
    export_list_t exports;
    bool isLoading;
    bool isEmitted;
    struct module_record_t * next;
} module_record_t;

typedef struct {
    char * chars;
    size_t length;
    size_t capacity;
} source_buffer_t;

typedef struct {
    module_record_t * modules;
    char ** loadingStack;
    size_t loadingStackCount;
    size_t loadingStackCapacity;
    source_buffer_t stitchedSource;
    bool hadError;
} module_context_t;

static void module_loader_append_source(source_buffer_t *, char const *, size_t);
static char * module_loader_canonicalize_path(char const *);
static void module_loader_cleanup_context(module_context_t *);
static void module_loader_cleanup_imports(import_spec_t *, size_t);
static module_record_t * module_loader_find_module(module_context_t *, char const *);
static void module_loader_free_module(module_record_t *);
static char * module_loader_join_paths(char const *, char const *);
static bool module_loader_parse_export(char const *, char **);
static bool module_loader_parse_import(char const *, import_spec_t *);
static bool module_loader_parse_module(char const *, module_record_t *, import_spec_t **, size_t *, char **);
static module_record_t * module_loader_process_module(module_context_t *, char const *);
static module_record_t * module_loader_process_module_from(module_context_t *, char const *, char const *);
static char * module_loader_read_file(char const *);
static char * module_loader_resolve_import_path(char const *, char const *);
static bool module_loader_starts_with_keyword(char const *, char const *);
static char * module_loader_strdup(char const *);
static char * module_loader_substr(char const *, size_t);
static bool module_loader_validate_named_imports(module_context_t *, module_record_t *, import_spec_t const *,
                                                 char const *);
static void module_loader_error(module_context_t *, char const *, ...);

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

    module_loader_append_source(&context.stitchedSource, "\0", 1);
    char * result = context.stitchedSource.chars;
    context.stitchedSource.chars = NULL;
    context.stitchedSource.length = 0;
    context.stitchedSource.capacity = 0;

    module_loader_cleanup_context(&context);
    return result;
}

static void module_loader_append_source(source_buffer_t * buffer, char const * text, size_t length) {
    if (!length) {
        return;
    }

    size_t required = buffer->length + length;
    if (required > buffer->capacity) {
        size_t newCapacity = buffer->capacity ? buffer->capacity : 128;
        while (newCapacity < required) {
            newCapacity *= 2;
        }
        char * grown = realloc(buffer->chars, newCapacity);
        if (!grown) {
            fprintf(stderr, "Module error: Failed to allocate module source buffer\n");
            exit(EXIT_CODE_SYSTEM_ERROR);
        }
        buffer->chars = grown;
        buffer->capacity = newCapacity;
    }

    memcpy(buffer->chars + buffer->length, text, length);
    buffer->length += length;
}

static char * module_loader_canonicalize_path(char const * path) {
#if defined(_WIN32)
    char * resolved = _fullpath(NULL, path, 0);
    if (!resolved) {
        return NULL;
    }
    // _fullpath does not check whether the file exists; verify before returning
    FILE * check = fopen(resolved, "rb");
    if (!check) {
        free(resolved);
        return NULL;
    }
    fclose(check);
    return resolved;
#else
    return realpath(path, NULL);
#endif
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

static void module_loader_cleanup_imports(import_spec_t * imports, size_t importCount) {
    for (size_t i = 0; i < importCount; i++) {
        free(imports[i].path);
        for (size_t j = 0; j < imports[i].importedNameCount; j++) {
            free(imports[i].importedNames[j]);
        }
        free(imports[i].importedNames);
    }
    free(imports);
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
    for (size_t i = 0; i < module->exports.count; i++) {
        free(module->exports.names[i]);
    }
    free(module->exports.names);
    free(module);
}

static char * module_loader_join_paths(char const * left, char const * right) {
    size_t leftLength = strlen(left);
    size_t rightLength = strlen(right);
    size_t hasSeparator = leftLength && left[leftLength - 1] == PATH_SEPARATOR;
    size_t totalLength = leftLength + rightLength + (hasSeparator ? 0 : 1) + 1;
    char * joined = malloc(totalLength);
    if (!joined) {
        return NULL;
    }

    memcpy(joined, left, leftLength);
    size_t offset = leftLength;
    if (!hasSeparator) {
        joined[offset++] = PATH_SEPARATOR;
    }
    memcpy(joined + offset, right, rightLength);
    joined[offset + rightLength] = '\0';
    return joined;
}

static bool module_loader_parse_export(char const * line, char ** exportName) {
    char const * cursor = line;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (!module_loader_starts_with_keyword(cursor, "export")) {
        return false;
    }
    cursor += strlen("export");
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (module_loader_starts_with_keyword(cursor, "var")) {
        cursor += strlen("var");
    } else if (module_loader_starts_with_keyword(cursor, "fun")) {
        cursor += strlen("fun");
    } else if (module_loader_starts_with_keyword(cursor, "class")) {
        cursor += strlen("class");
    } else {
        return false;
    }

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    char const * identifierStart = cursor;
    if (!((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') || *cursor == '_')) {
        return false;
    }
    cursor++;
    while ((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') ||
           (*cursor >= '0' && *cursor <= '9') || *cursor == '_') {
        cursor++;
    }

    *exportName = module_loader_substr(identifierStart, (size_t)(cursor - identifierStart));
    return *exportName != NULL;
}

static bool module_loader_parse_import(char const * line, import_spec_t * importSpec) {
    memset(importSpec, 0, sizeof(*importSpec));

    char const * cursor = line;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (!module_loader_starts_with_keyword(cursor, "import")) {
        return false;
    }
    cursor += strlen("import");
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (*cursor == '"') {
        cursor++;
        char const * pathStart = cursor;
        while (*cursor && *cursor != '"') {
            cursor++;
        }
        if (*cursor != '"') {
            return false;
        }
        importSpec->path = module_loader_substr(pathStart, (size_t)(cursor - pathStart));
        if (!importSpec->path) {
            return false;
        }
        cursor++;
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
        return *cursor == ';';
    }

    if (*cursor != '{') {
        return false;
    }
    cursor++;

    for (;;) {
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        char const * nameStart = cursor;
        if (!((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') || *cursor == '_')) {
            module_loader_cleanup_imports(importSpec, 1);
            return false;
        }
        cursor++;
        while ((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') ||
               (*cursor >= '0' && *cursor <= '9') || *cursor == '_') {
            cursor++;
        }

        char * name = module_loader_substr(nameStart, (size_t)(cursor - nameStart));
        if (!name) {
            module_loader_cleanup_imports(importSpec, 1);
            return false;
        }

        size_t newCapacity = importSpec->importedNameCount + 1;
        char ** grownNames = realloc(importSpec->importedNames, newCapacity * sizeof(char *));
        if (!grownNames) {
            free(name);
            module_loader_cleanup_imports(importSpec, 1);
            return false;
        }
        importSpec->importedNames = grownNames;
        importSpec->importedNames[importSpec->importedNameCount++] = name;

        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        if (*cursor == ',') {
            cursor++;
            continue;
        }
        if (*cursor == '}') {
            cursor++;
            break;
        }

        module_loader_cleanup_imports(importSpec, 1);
        return false;
    }

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (!module_loader_starts_with_keyword(cursor, "from")) {
        module_loader_cleanup_imports(importSpec, 1);
        return false;
    }
    cursor += strlen("from");

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (*cursor != '"') {
        module_loader_cleanup_imports(importSpec, 1);
        return false;
    }

    cursor++;
    char const * pathStart = cursor;
    while (*cursor && *cursor != '"') {
        cursor++;
    }
    if (*cursor != '"') {
        module_loader_cleanup_imports(importSpec, 1);
        return false;
    }

    importSpec->path = module_loader_substr(pathStart, (size_t)(cursor - pathStart));
    if (!importSpec->path) {
        module_loader_cleanup_imports(importSpec, 1);
        return false;
    }

    cursor++;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (*cursor != ';') {
        module_loader_cleanup_imports(importSpec, 1);
        return false;
    }

    return true;
}

static bool module_loader_parse_module(char const * source, module_record_t * module, import_spec_t ** imports,
                                       size_t * importCount, char ** transformedSource) {
    *imports = NULL;
    *importCount = 0;
    *transformedSource = NULL;

    source_buffer_t transformed;
    transformed.chars = NULL;
    transformed.length = 0;
    transformed.capacity = 0;

    char const * lineStart = source;
    while (*lineStart) {
        char const * lineEnd = lineStart;
        while (*lineEnd && *lineEnd != '\n') {
            lineEnd++;
        }

        size_t lineLength = (size_t)(lineEnd - lineStart);
        bool hasLineBreak = *lineEnd == '\n';
        char * line = module_loader_substr(lineStart, lineLength);
        if (!line) {
            free(transformed.chars);
            return false;
        }

        import_spec_t importSpec;
        if (module_loader_parse_import(line, &importSpec)) {
            import_spec_t * grownImports = realloc(*imports, ((*importCount) + 1) * sizeof(import_spec_t));
            if (!grownImports) {
                free(line);
                module_loader_cleanup_imports(&importSpec, 1);
                module_loader_cleanup_imports(*imports, *importCount);
                free(transformed.chars);
                return false;
            }
            *imports = grownImports;
            (*imports)[(*importCount)++] = importSpec;
            if (hasLineBreak) {
                module_loader_append_source(&transformed, "\n", 1);
            }
        } else {
            char * exportName = NULL;
            if (module_loader_parse_export(line, &exportName)) {
                bool duplicateExport = false;
                for (size_t i = 0; i < module->exports.count; i++) {
                    if (!strcmp(module->exports.names[i], exportName)) {
                        duplicateExport = true;
                        break;
                    }
                }
                if (duplicateExport) {
                    free(exportName);
                    free(line);
                    module_loader_cleanup_imports(*imports, *importCount);
                    free(transformed.chars);
                    return false;
                }

                if (module->exports.count + 1 > module->exports.capacity) {
                    size_t newCapacity = module->exports.capacity ? module->exports.capacity * 2 : 8;
                    char ** grownExports = realloc(module->exports.names, newCapacity * sizeof(char *));
                    if (!grownExports) {
                        free(exportName);
                        free(line);
                        module_loader_cleanup_imports(*imports, *importCount);
                        free(transformed.chars);
                        return false;
                    }
                    module->exports.names = grownExports;
                    module->exports.capacity = newCapacity;
                }
                module->exports.names[module->exports.count++] = exportName;

                char const * trimmed = line;
                while (*trimmed == ' ' || *trimmed == '\t') {
                    trimmed++;
                }
                size_t indentLength = (size_t)(trimmed - line);
                module_loader_append_source(&transformed, line, indentLength);
                module_loader_append_source(&transformed, trimmed + strlen("export"), strlen(trimmed) - strlen("export"));
            } else {
                module_loader_append_source(&transformed, line, lineLength);
            }
            if (hasLineBreak) {
                module_loader_append_source(&transformed, "\n", 1);
            }
        }

        free(line);
        lineStart = *lineEnd ? lineEnd + 1 : lineEnd;
    }

    module_loader_append_source(&transformed, "\0", 1);
    *transformedSource = transformed.chars;
    return true;
}

static module_record_t * module_loader_process_module(module_context_t * context, char const * rawPath) {
    return module_loader_process_module_from(context, rawPath, NULL);
}

static module_record_t * module_loader_process_module_from(module_context_t * context, char const * rawPath,
                                                           char const * importerPath) {
    char * canonicalPath = module_loader_canonicalize_path(rawPath);
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

    char * source = module_loader_read_file(module->canonicalPath);
    if (!source) {
        module_loader_error(context, "Could not read module \"%s\"", module->canonicalPath);
        module->isLoading = false;
        context->loadingStackCount--;
        return NULL;
    }

    import_spec_t * imports = NULL;
    size_t importCount = 0;
    char * transformedSource = NULL;
    if (!module_loader_parse_module(source, module, &imports, &importCount, &transformedSource)) {
        free(source);
        module_loader_error(context, "Invalid import or export declaration in \"%s\"", module->canonicalPath);
        module_loader_cleanup_imports(imports, importCount);
        module->isLoading = false;
        context->loadingStackCount--;
        return NULL;
    }
    free(source);

    for (size_t i = 0; i < importCount; i++) {
        char * resolvedImportPath = module_loader_resolve_import_path(module->canonicalPath, imports[i].path);
        if (!resolvedImportPath) {
            module_loader_error(context, "Could not resolve import path \"%s\" from \"%s\"", imports[i].path,
                                module->canonicalPath);
            module_loader_cleanup_imports(imports, importCount);
            free(transformedSource);
            module->isLoading = false;
            context->loadingStackCount--;
            return NULL;
        }

        module_record_t * importedModule =
            module_loader_process_module_from(context, resolvedImportPath, module->canonicalPath);
        free(resolvedImportPath);
        if (!importedModule) {
            module_loader_cleanup_imports(imports, importCount);
            free(transformedSource);
            module->isLoading = false;
            context->loadingStackCount--;
            return NULL;
        }

        if (!module_loader_validate_named_imports(context, importedModule, &imports[i], module->canonicalPath)) {
            module_loader_cleanup_imports(imports, importCount);
            free(transformedSource);
            module->isLoading = false;
            context->loadingStackCount--;
            return NULL;
        }
    }

    module_loader_cleanup_imports(imports, importCount);

    module_loader_append_source(&context->stitchedSource, transformedSource, strlen(transformedSource));

    free(transformedSource);

    module->isLoading = false;
    module->isEmitted = true;
    context->loadingStackCount--;
    return module;
}

static char * module_loader_read_file(char const * path) {
    FILE * file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char * buffer = malloc(fileSize + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    fclose(file);
    if (bytesRead < fileSize) {
        free(buffer);
        return NULL;
    }

    buffer[bytesRead] = '\0';
    return buffer;
}

static char * module_loader_resolve_import_path(char const * importerPath, char const * importPath) {
    if (!importPath || !importPath[0]) {
        return NULL;
    }

    bool isAbsolute = importPath[0] == '/' || importPath[0] == '\\';
#if defined(_WIN32)
    if (strlen(importPath) > 1 && importPath[1] == ':') {
        isAbsolute = true;
    }
#endif

    if (isAbsolute) {
        return module_loader_strdup(importPath);
    }

    char const * lastSeparator = strrchr(importerPath, '/');
#if defined(_WIN32)
    char const * windowsSeparator = strrchr(importerPath, '\\');
    if (!lastSeparator || (windowsSeparator && windowsSeparator > lastSeparator)) {
        lastSeparator = windowsSeparator;
    }
#endif

    if (!lastSeparator) {
        return module_loader_strdup(importPath);
    }

    size_t directoryLength = (size_t)(lastSeparator - importerPath);
    char * directory = module_loader_substr(importerPath, directoryLength);
    if (!directory) {
        return NULL;
    }

    char * resolvedPath = module_loader_join_paths(directory, importPath);
    free(directory);
    return resolvedPath;
}

static bool module_loader_starts_with_keyword(char const * text, char const * keyword) {
    size_t keywordLength = strlen(keyword);
    if (strncmp(text, keyword, keywordLength)) {
        return false;
    }

    char c = text[keywordLength];
    bool identifierCharacter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    return !identifierCharacter;
}

static char * module_loader_strdup(char const * text) {
    size_t length = strlen(text);
    char * copy = malloc(length + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, text, length + 1);
    return copy;
}

static char * module_loader_substr(char const * text, size_t length) {
    char * copy = malloc(length + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, text, length);
    copy[length] = '\0';
    return copy;
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

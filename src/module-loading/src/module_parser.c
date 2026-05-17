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
 * @file module_parser.c
 * @brief Parser for Cellox module source text.
 * @details Parses import and export declarations line-by-line, strips export
 * keywords from the emitted source, and returns the metadata needed by the
 * module loader for dependency validation.
 */

#include <module-loading/internal/module_parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/string_utils.h"

/**
 * @brief Frees the dynamically allocated members of a single import spec.
 * @param importSpec The import specification to clean up.
 */
static void module_parser_cleanup_import_spec(import_spec_t * importSpec) {
    free(importSpec->path);
    importSpec->path = NULL;
    for (size_t j = 0; j < importSpec->importedNameCount; j++) {
        free(importSpec->importedNames[j]);
    }
    free(importSpec->importedNames);
    importSpec->importedNames = NULL;
    importSpec->importedNameCount = 0;
}

/**
 * @brief Checks whether a token starts with a standalone keyword.
 * @details The match only succeeds if the keyword is followed by a
 * non-identifier character so partial matches like `important` do not count
 * as `import`.
 * @param text The token sequence to inspect.
 * @param keyword The keyword to compare against.
 * @return true if `text` starts with `keyword` as a standalone token.
 */
static bool module_parser_starts_with_keyword(char const * text, char const * keyword) {
    size_t keywordLength = strlen(keyword);
    if (strncmp(text, keyword, keywordLength)) {
        return false;
    }
    char c = text[keywordLength];
    bool identifierChar = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    return !identifierChar;
}

/**
 * @brief Parses a single export declaration.
 * @param line Source line to inspect.
 * @param exportName Out-parameter receiving the exported identifier.
 * @return true if `line` contains a supported export declaration, otherwise false.
 */
static bool module_parser_parse_export(char const * line, char ** exportName) {
    char const * cursor = line;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (!module_parser_starts_with_keyword(cursor, "export")) {
        return false;
    }
    cursor += strlen("export");
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (module_parser_starts_with_keyword(cursor, "var")) {
        cursor += strlen("var");
    } else if (module_parser_starts_with_keyword(cursor, "fun")) {
        cursor += strlen("fun");
    } else if (module_parser_starts_with_keyword(cursor, "class")) {
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

    *exportName = string_utils_substr(identifierStart, (size_t)(cursor - identifierStart));
    return *exportName != NULL;
}

/**
 * @brief Parses a single import declaration.
 * @details Supports both bare-path imports and named imports. Any partial
 * allocation performed while parsing is released before returning false.
 * @param line Source line to inspect.
 * @param importSpec Out-parameter receiving the parsed import metadata.
 * @return true if `line` contains a valid import declaration, otherwise false.
 */
static bool module_parser_parse_import(char const * line, import_spec_t * importSpec) {
    memset(importSpec, 0, sizeof(*importSpec));

    char const * cursor = line;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    if (!module_parser_starts_with_keyword(cursor, "import")) {
        return false;
    }
    cursor += strlen("import");
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }

    // Bare-path import: import "path/to/module";
    if (*cursor == '"') {
        cursor++;
        char const * pathStart = cursor;
        while (*cursor && *cursor != '"') {
            cursor++;
        }
        if (*cursor != '"') {
            return false;
        }
        importSpec->path = string_utils_substr(pathStart, (size_t)(cursor - pathStart));
        if (!importSpec->path) {
            return false;
        }
        cursor++;
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
        return *cursor == ';';
    }

    // Named import: import { A, B } from "path/to/module";
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
            module_parser_cleanup_import_spec(importSpec);
            return false;
        }
        cursor++;
        while ((*cursor >= 'a' && *cursor <= 'z') || (*cursor >= 'A' && *cursor <= 'Z') ||
               (*cursor >= '0' && *cursor <= '9') || *cursor == '_') {
            cursor++;
        }

        char * name = string_utils_substr(nameStart, (size_t)(cursor - nameStart));
        if (!name) {
            module_parser_cleanup_import_spec(importSpec);
            return false;
        }

        char ** grownNames = realloc(importSpec->importedNames, (importSpec->importedNameCount + 1) * sizeof(char *));
        if (!grownNames) {
            free(name);
            module_parser_cleanup_import_spec(importSpec);
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

        module_parser_cleanup_import_spec(importSpec);
        return false;
    }

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (!module_parser_starts_with_keyword(cursor, "from")) {
        module_parser_cleanup_import_spec(importSpec);
        return false;
    }
    cursor += strlen("from");

    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (*cursor != '"') {
        module_parser_cleanup_import_spec(importSpec);
        return false;
    }

    cursor++;
    char const * pathStart = cursor;
    while (*cursor && *cursor != '"') {
        cursor++;
    }
    if (*cursor != '"') {
        module_parser_cleanup_import_spec(importSpec);
        return false;
    }

    importSpec->path = string_utils_substr(pathStart, (size_t)(cursor - pathStart));
    if (!importSpec->path) {
        module_parser_cleanup_import_spec(importSpec);
        return false;
    }

    cursor++;
    while (*cursor == ' ' || *cursor == '\t') {
        cursor++;
    }
    if (*cursor != ';') {
        module_parser_cleanup_import_spec(importSpec);
        return false;
    }

    return true;
}

/**
 * @brief Appends text to a growable source buffer.
 * @param buffer Destination buffer to grow and append into.
 * @param text Text bytes to append.
 * @param length Number of bytes to append from `text`.
 */
void module_parser_append_source(source_buffer_t * buffer, char const * text, size_t length) {
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

/**
 * @brief Parses a module source file into imports, exports, and transformed source.
 * @param source NUL-terminated module source text.
 * @param exports Output export list populated from `export` declarations.
 * @param imports Output array of import specifications allocated on success.
 * @param importCount Output count of entries stored in `imports`.
 * @param transformedSource Output transformed source with `export` stripped.
 * @return true on success, otherwise false.
 */
bool module_parser_parse(char const * source, export_list_t * exports, import_spec_t ** imports, size_t * importCount,
                         char ** transformedSource) {
    *imports = NULL;
    *importCount = 0;
    *transformedSource = NULL;

    source_buffer_t transformed = {NULL, 0, 0};

    char const * lineStart = source;
    while (*lineStart) {
        char const * lineEnd = lineStart;
        while (*lineEnd && *lineEnd != '\n') {
            lineEnd++;
        }

        size_t lineLength = (size_t)(lineEnd - lineStart);
        bool hasLineBreak = *lineEnd == '\n';
        char * line = string_utils_substr(lineStart, lineLength);
        if (!line) {
            free(transformed.chars);
            return false;
        }

        import_spec_t importSpec;
        if (module_parser_parse_import(line, &importSpec)) {
            import_spec_t * grownImports = realloc(*imports, ((*importCount) + 1) * sizeof(import_spec_t));
            if (!grownImports) {
                free(line);
                module_parser_cleanup_imports(&importSpec, 1);
                module_parser_cleanup_imports(*imports, *importCount);
                *imports = NULL;
                *importCount = 0;
                free(transformed.chars);
                return false;
            }
            *imports = grownImports;
            (*imports)[(*importCount)++] = importSpec;
            if (hasLineBreak) {
                module_parser_append_source(&transformed, "\n", 1);
            }
        } else {
            char * exportName = NULL;
            if (module_parser_parse_export(line, &exportName)) {
                bool duplicateExport = false;
                for (size_t i = 0; i < exports->count; i++) {
                    if (!strcmp(exports->names[i], exportName)) {
                        duplicateExport = true;
                        break;
                    }
                }
                if (duplicateExport) {
                    free(exportName);
                    free(line);
                    module_parser_cleanup_imports(*imports, *importCount);
                    *imports = NULL;
                    *importCount = 0;
                    free(transformed.chars);
                    return false;
                }

                if (exports->count + 1 > exports->capacity) {
                    size_t newCapacity = exports->capacity ? exports->capacity * 2 : 8;
                    char ** grownExports = realloc(exports->names, newCapacity * sizeof(char *));
                    if (!grownExports) {
                        free(exportName);
                        free(line);
                        module_parser_cleanup_imports(*imports, *importCount);
                        *imports = NULL;
                        *importCount = 0;
                        free(transformed.chars);
                        return false;
                    }
                    exports->names = grownExports;
                    exports->capacity = newCapacity;
                }
                exports->names[exports->count++] = exportName;

                char const * trimmed = line;
                while (*trimmed == ' ' || *trimmed == '\t') {
                    trimmed++;
                }
                size_t indentLength = (size_t)(trimmed - line);
                module_parser_append_source(&transformed, line, indentLength);
                module_parser_append_source(&transformed, trimmed + strlen("export"),
                                            strlen(trimmed) - strlen("export"));
            } else {
                module_parser_append_source(&transformed, line, lineLength);
            }
            if (hasLineBreak) {
                module_parser_append_source(&transformed, "\n", 1);
            }
        }

        free(line);
        lineStart = *lineEnd ? lineEnd + 1 : lineEnd;
    }

    module_parser_append_source(&transformed, "\0", 1);
    *transformedSource = transformed.chars;
    return true;
}

/**
 * @brief Frees an array of import specifications.
 * @param imports Import array previously returned by module_parser_parse().
 * @param importCount Number of entries stored in `imports`.
 */
void module_parser_cleanup_imports(import_spec_t * imports, size_t importCount) {
    for (size_t i = 0; i < importCount; i++) {
        module_parser_cleanup_import_spec(imports + i);
    }
    free(imports);
}

/**
 * @brief Frees all export names stored in an export list.
 * @param list Export list to reset and release.
 */
void module_parser_export_list_free(export_list_t * list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->names[i]);
    }
    free(list->names);
    list->names = NULL;
    list->count = 0;
    list->capacity = 0;
}

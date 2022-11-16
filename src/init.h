#ifndef CELLOX_INIT_H_
#define CELLOX_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define CELLOX_USAGE_MESSAGE ("Usage: Cellox ((-h|--help|-v|--version) | ((-scf | --store-chunk-file) |(-rcf |--read-chunk-file)) [path]\n")

void init_repl();
void init_run_from_file(char const *, bool);
void init_show_help();
void init_show_version();

#ifdef __cplusplus
}
#endif

#endif

#ifndef CELLOX_INIT_H_
#define CELLOX_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void init_repl();
void init_run_from_file(char const *, bool);

#ifdef __cplusplus
}
#endif

#endif

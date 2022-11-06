#ifndef CELLOX_INIT_H_
#define CELLOX_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Initialization method of the interpreter
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
void init_initialize(int const argc, char const ** argv);

#ifdef __cplusplus
}
#endif

#endif
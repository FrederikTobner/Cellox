#include "init.h"

/// @brief Main entry point of the cellox interpreter
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
/// @return 0 -> OK, 64 -> wrong usage, 70 -> runtime error, 74 error reading from file, 80 memory allocation error
int main(int argc, char const ** argv)
{
    init_initialize(argc, argv);
    return EXIT_CODE_OK;
}
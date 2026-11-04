#ifndef DISASSEMBLER_DISASSEMBLER_H_
#define DISASSEMBLER_DISASSEMBLER_H_

/// @brief Disassembles a given Cellox file or chunk file.
/// @param filePath The path to the file to disassemble.
void disassembler_disassemble_file(char const * filePath);

/// @brief Shows the usage information for the disassembler.
void disassembler_show_usage(void);

#endif
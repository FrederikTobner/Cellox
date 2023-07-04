#include "disassembler.h"

int main(int argc, char const ** argv) {
    if (argc != 2) {
        disassembler_show_usage();
    }
    disassembler_disassemble_file(argv[1]);
    return 0;
}

# Disassembler Tool

**CMake target:** `CelloxDisassembler`

This directory builds the Cellox disassembler executable. The tool reads Cellox
source or bytecode-oriented inputs and prints a human-readable view of the
generated instructions for debugging and inspection.

At a high level, this tool exists to make the compiler output easier to inspect
when debugging frontend, bytecode, or optimizer behavior.

# clx_module_loading — Module graph resolution

**CMake target:** `clx_module_loading`

`clx_module_loading` is responsible for Cellox source-level module handling. It
loads source files, resolves import paths, validates import/export usage, and
assembles the final source text that is passed into the compiler frontend.

At a high level, this module turns a file-based module graph into a compiler
input string while keeping filesystem handling and import parsing separate from
the frontend itself.

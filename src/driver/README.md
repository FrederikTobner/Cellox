# clx_driver — Top-level compiler orchestration

**CMake target:** `clx_driver`

`clx_driver` is the application-facing orchestration layer of Cellox. It sits
above the frontend, runtime, and module-loading code and turns high-level user
actions into concrete compiler/runtime behavior.

At a high level, this module is responsible for:

- parsing command-line arguments,
- selecting the requested execution mode,
- wiring together the frontend and runtime for normal program execution.

The main executable adds only the actual `main()` entry point on top of this
library.

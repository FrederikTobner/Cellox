# clx_utils — Small shared helper routines

**CMake target:** `clx_utils`

`clx_utils` contains small reusable helper functions that do not belong to a
larger runtime or compiler subsystem. Right now it is primarily the string
utility layer used by parsing, module loading, and parts of the runtime.

At a high level, this module exists to keep generic helpers out of the more
domain-specific libraries.

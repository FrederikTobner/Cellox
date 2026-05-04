# clx_os — Operating-system abstraction layer

**CMake target:** `clx_os`

`clx_os` provides the small portability layer used when Cellox needs to talk to
the host operating system. Exactly one OS-specific implementation is selected at
configure time and exposed through a stable public API.

At a high level, this module isolates platform-specific filesystem, path,
stdio, temporary-file, and timing behavior from the rest of the project.

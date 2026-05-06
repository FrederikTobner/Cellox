# clx_os — Operating-system abstraction layer

**CMake target:** `clx_os`

`clx_os` provides the small portability layer used when Cellox needs to talk to
the host operating system. Exactly one OS-specific implementation is selected at
configure time and exposed through a stable public API.

At a high level, this module isolates platform-specific filesystem, path,
stdio, temporary-file, and timing behaviour from the rest of the project.

## Directory layout

```
conditionals/os/
├── include/clx_os/       — public headers (stable API consumed by all targets)
│   ├── fs.h              — filesystem helpers
│   ├── path.h            — path manipulation
│   ├── platform.h        — platform name query
│   ├── stdio.h           — stdio wrappers
│   ├── temp.h            — temporary-file utilities
│   └── time.h            — wall-clock timing
├── unix/
│   ├── impl/             — shared POSIX implementations (Linux + macOS)
│   │   ├── fs.c, path.c, stdio.c, temp.c, time.c
│   ├── linux/impl/
│   │   └── platform.c    — platform_name → "linux"
│   └── macos/impl/
│       └── platform.c    — platform_name → "macos"
└── windows/impl/         — Windows-specific implementations
    ├── fs.c, path.c, platform.c, stdio.c, temp.c, time.c
```

The cmake selection logic lives in `cmake/conditionals/os/`; the appropriate
`CLX_OS_SOURCES` list is appended there and consumed by `os/CMakeLists.txt`.

## Public API

### `clx_os/path.h`

| Function | Description |
|----------|-------------|
| `clx_os_path_canonicalize(path)` | Return a heap-allocated canonicalised absolute path. |
| `clx_os_path_is_absolute(path)` | Return true if the path is absolute. |
| `clx_os_path_find_last_separator(path)` | Return a pointer to the last path separator, or NULL. |
| `clx_os_path_separator()` | Return the platform path separator character (`/` or `\`). |
| `clx_os_path_executable_dir()` | Return the directory of the running executable (cached; caller does not free). |

### `clx_os/fs.h`

| Function | Description |
|----------|-------------|
| `clx_os_fs_ensure_directory(path)` | Create the directory and any missing parents; return true on success. |
| `clx_os_fs_path_exists(path)` | Return true if the path exists (file or directory). |

### `clx_os/platform.h`

| Function | Description |
|----------|-------------|
| `clx_os_platform_name()` | Return a string literal identifying the host OS (`"linux"`, `"macos"`, `"windows"`). |

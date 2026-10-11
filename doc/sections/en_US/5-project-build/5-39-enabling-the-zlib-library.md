## Enabling the zlib library

[zlib](https://zlib.net/) is a mandatory dependency of this library: the
[darknetxx](https://github.com/yuriysydor1991/darknetxx) dxxwjz files the
library reads are the gzip compressed JSON documents, and the ZlibController reads the gzip compressed dxxwjz documents of the darknetxx layouts through it, a chunk at a time. The
`ZlibController` of [src/zlib](/src/zlib) is the zlib controller component of
the [cpp-app-template](https://github.com/yuriysydor1991/cpp-app-template)
(its `appZlib` branch), in the adaptation the darknetxx made of it for its
dxxwjz files: the in memory and the gzip file compression and the gzip file
streams, which read and write the files of the hundreds of megabytes a chunk at
a time. The `ENABLE_ZLIB` CMake option therefore defaults to `ON` and there is
nothing to switch on for a normal build:

```
# Inside the source root directory

cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix
```

Setting `ENABLE_ZLIB=OFF` stops the configure with an error, since the library
does not build without the zlib.

The zlib integration is split into two CMake modules:

- [cmake/enablers/template-project-zlib-enabler.cmake](/cmake/enablers/template-project-zlib-enabler.cmake)
  only makes the dependency available. It runs before `src/` is added, so the
  unit test executables declared inside the source tree can already link
  `ZLIB::ZLIB` themselves.
- [cmake/enablers/template-project-zlib-linker.cmake](/cmake/enablers/template-project-zlib-linker.cmake)
  runs after `src/` and links the zlib against the library target, which only
  exists at that point. The link is `PRIVATE`: no installable public header
  exposes a zlib type, so the dependency does not leak into the exported
  interface of the library.

The enabler first probes the system wide zlib with `find_package`. When nothing
is found it falls back to fetching the sources with `FetchContent` from the
repository and tag held in the `TEMPLATE_APP_ZLIB_GIT` and
`TEMPLATE_APP_ZLIB_GIT_TAG` cache variables (the `v1.3.1` tag by default), and
links the static `zlibstatic` library it builds under the same `ZLIB::ZLIB`
name:

```
# Inside the source root directory

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DTEMPLATE_APP_ZLIB_GIT_TAG=v1.3.1
```

Installing the system development package (for example `zlib1g-dev` on Debian
based distributions) is the faster route and keeps the configure stage
offline.

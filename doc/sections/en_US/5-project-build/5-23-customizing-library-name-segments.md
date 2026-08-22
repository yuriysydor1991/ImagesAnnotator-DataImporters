## Customizing the installable library name segments

The library name that downstream consumers see (the shared object, the
`include/<name>/` subdirectory holding the public headers, the CMake package
directory and the imported target namespace exported through
`install(EXPORT ...)`) is derived from three optional CMake options. They let
parallel installs of distinct versions of the library coexist on the same host
- for example `include/ImagesAnnotatorDataImporters-0.13.0-dev/` next to
`include/ImagesAnnotatorDataImporters-0.13/`.

| Option | Default | Effect |
|---|---|---|
| `-DLIB_INCLUDE_MINOR_IN_NAME=OFF` | `ON` | Appends `.<minor>` to the library name |
| `-DLIB_INCLUDE_MICRO_IN_NAME=ON` | `OFF` | Appends `.<micro>` (implies the minor flag) |
| `-DLIB_NAME_SUFFIX=-dev` | `""` | Appends an arbitrary trailing tag |

The minor segment is included by default because the public API namespace
carries the major and the minor version pair (`ImagesAnnotatorDataImporters013`)
and because the [data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md)
names itself the very same way. Two minor releases therefore install completely
side by side - their binaries, their header directories and their CMake packages
all differ.

Resulting names for this `0.13.0` project:

| Configure flags | Library name | Produced binary |
|---|---|---|
| (none) | `ImagesAnnotatorDataImporters-0.13` | `libImagesAnnotatorDataImporters-0.13.so` |
| `-DLIB_INCLUDE_MINOR_IN_NAME=OFF` | `ImagesAnnotatorDataImporters-0` | `libImagesAnnotatorDataImporters-0.so` |
| `-DLIB_INCLUDE_MICRO_IN_NAME=ON` | `ImagesAnnotatorDataImporters-0.13.0` | `libImagesAnnotatorDataImporters-0.13.0.so` |
| `-DLIB_NAME_SUFFIX=-dev` | `ImagesAnnotatorDataImporters-0.13-dev` | `libImagesAnnotatorDataImporters-0.13-dev.so` |
| `-DLIB_INCLUDE_MICRO_IN_NAME=ON -DLIB_NAME_SUFFIX=-dev` | `ImagesAnnotatorDataImporters-0.13.0-dev` | `libImagesAnnotatorDataImporters-0.13.0-dev.so` |

Combined configure example:

```
# inside the project root directory

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DLIB_INCLUDE_MICRO_IN_NAME=ON \
  -DLIB_NAME_SUFFIX=-dev
cmake --build build -j$(nproc)
cmake --install build --prefix /usr/local
```

The same name is used consistently for every installed artefact. With the
default configure that gives:

- `<prefix>/lib/libImagesAnnotatorDataImporters-0.13.so.0.13.0` with the
  `libImagesAnnotatorDataImporters-0.13.so.0` soname symlink (the `SOVERSION`
  stays the major version) and the development `.so` namelink,
- `<prefix>/include/ImagesAnnotatorDataImporters-0.13/*.h` - the public headers of
  [src/lib/facade/public](/src/lib/facade/public), declared through the
  `INSTALL_INTERFACE` of the library target,
- `<libdir>/cmake/ImagesAnnotatorDataImporters-0.13/` with the generated
  `Config.cmake`, `ConfigVersion.cmake` and `Targets.cmake` files, plus the
  `ImagesAnnotatorDataImporters-0.13::ImagesAnnotatorDataImporters-0.13` imported
  target.

A consumer therefore writes `find_package(ImagesAnnotatorDataImporters-0.13 0.13
REQUIRED)` for a default install, and has to use the customised name verbatim
when the segments were changed. The C++ namespace of the sources,
`ImagesAnnotatorDataImporters013`, is not affected by these options.

The derivation lives in
[cmake/template-project-misc-variables-declare.cmake](/cmake/template-project-misc-variables-declare.cmake)
and is published as the `PROJECT_LIBRARY_NAME` CMake cache entry that the rest
of the build system consumes, including the Doxygen HTML output directory name.
Enabling `LIB_INCLUDE_MICRO_IN_NAME` without `LIB_INCLUDE_MINOR_IN_NAME`
triggers a configure-time warning and the minor flag is enabled implicitly to
keep the produced name syntactically valid.

The name of the [data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md)
is independent from these options: it is chosen with the separate
`IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE` cache variable.

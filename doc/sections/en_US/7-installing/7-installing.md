# Installing

Execute the install command from the project's build directory. The project
installs no executable: what lands in the prefix is the shared library, its
public headers and the CMake package that lets other projects find them.

## Default installation

Without an explicit prefix CMake uses the platform default, usually
`/usr/local` on the Unix-like systems, which needs the administrator rights
(GNU/Linux based):

```
# from the project build directory

sudo cmake --install .
```

## Installation by the quick build scripts

The scripts of the [Quick build scripts](/doc/sections/en_US/5-project-build/5-38-quick-build-scripts.md) section perform the install step themselves when the `--install` parameter is given, so the whole configure, build and install cycle takes the single command:

```
# inside the project root directory

scripts/build/release.sh --install
```

The install step alone, against the already built `build/release` directory, is performed by its stage script:

```
# inside the project root directory

scripts/build/release-install.sh --install
```

Both of them install into the `/usr` prefix by the `sudo cmake --install` call, so the `sudo` password is asked for. Without the `--install` parameter the install step is skipped entirely and no password is ever asked.

## Custom installation path

Add the `--prefix` parameter to install elsewhere. Any writable path will do,
which is the convenient way to test the package before installing it system
wide:

```
# replace the /usr path with your own if needed
# run from the project's build directory

sudo cmake --install . --prefix "/usr"
```

## What gets installed

For the default `ImagesAnnotatorDataImporters-0.14` library name a prefix receives
exactly the following files:

```
<prefix>/lib/libImagesAnnotatorDataImporters-0.14.so.0.14.0
<prefix>/lib/libImagesAnnotatorDataImporters-0.14.so.0
<prefix>/lib/libImagesAnnotatorDataImporters-0.14.so

<prefix>/include/ImagesAnnotatorDataImporters-0.14/ImportersAPI.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/IImporter.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/IImageSizeFacility.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/IADataImportersLib.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/IADataImportersContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/IADataImportersFacade.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/CocoImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/CreateMLImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/PascalVocImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/PlainTxtImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/PyTorchImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/UltralyticsDetectImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/UltralyticsObbImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/UltralyticsSegmentImportContext.h
<prefix>/include/ImagesAnnotatorDataImporters-0.14/contexts/Yolo4ImportContext.h

<prefix>/lib/cmake/ImagesAnnotatorDataImporters-0.14/ImagesAnnotatorDataImporters-0.14Config.cmake
<prefix>/lib/cmake/ImagesAnnotatorDataImporters-0.14/ImagesAnnotatorDataImporters-0.14ConfigVersion.cmake
<prefix>/lib/cmake/ImagesAnnotatorDataImporters-0.14/ImagesAnnotatorDataImporters-0.14Targets.cmake
<prefix>/lib/cmake/ImagesAnnotatorDataImporters-0.14/ImagesAnnotatorDataImporters-0.14Targets-<config>.cmake
```

The `.so.0.14.0` file is the library itself, `.so.0` is the `SONAME` link the
dynamic linker resolves and the bare `.so` link is the development one the
compiler follows. The headers are the fifteen public ones of the
[src/lib/facade/public](/src/lib/facade/public) directory, described in
[The library's installable include header files](/doc/sections/en_US/4-project-structure/4-8-the-librarys-installable-include-header-files.md).

The four CMake files form the package that answers
`find_package(ImagesAnnotatorDataImporters-0.14 0.14 REQUIRED)`. The `Config` one
pulls the public `ImagesAnnotatorDataDrivers-0.12` dependency in before including
the exported targets, the `ConfigVersion` one makes the version request
succeed, and the `Targets` pair defines the
`ImagesAnnotatorDataImporters-0.14::ImagesAnnotatorDataImporters-0.14` imported
target. The `<config>` part of the last file name follows the build type, and
is `noconfig` when no `CMAKE_BUILD_TYPE` was set.

The library name is composed from the project name and its major and minor
version, so
all four path segments above change together if the name is customised, see
[Customizing library name segments](/doc/sections/en_US/5-project-build/5-23-customizing-library-name-segments.md).

## Install components

The installed files are split into two components. `Runtime` holds what a
program needs to run, that is the `.so.0.14.0` file and its `SONAME` link.
`Development` holds what a project needs to build against the library, that is
the bare `.so` link, the headers and the CMake package files. Either one may be
installed alone:

```
# from the project build directory

cmake --install . --component Runtime
cmake --install . --component Development
```

## Using the installed library

The library keeps the `ImagesAnnotatorDataDrivers-0.12` dependency public, so both
packages must be visible to the consuming project. Pass their prefixes through
`CMAKE_PREFIX_PATH`:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH="<importers prefix>;<data drivers prefix>"
```

If the library was installed outside the loader's default search path, refresh
the cache with `sudo ldconfig` or export `LD_LIBRARY_PATH=<prefix>/lib` before
starting the program that links it. The full walk-through is in
[Using the library in your project](/doc/sections/en_US/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md).

## Documentation install

If the project was configured to support the documentation install by a command
which looks like next:

```
# inside the project build directory

cmake ../ -DENABLE_DOC_DOXYGEN=ON -DDOXYGEN_DO_INSTALL=ON
```

then the generated HTML files are installed under `<prefix>/share/docs` as the
`Documentation` component by the same install command described in the
[Default installation](#default-installation) section. See
[Configuring the documentation install support](/doc/sections/en_US/5-project-build/documentation/5-5-configuring-the-documentation-install-support.md)
for the details.

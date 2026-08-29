## The ImagesAnnotatorDataDrivers dependency

The importers do not define their own record types. They build the annotations
database - the image records, their rectangles and the annotation names -
through the sibling
[ImagesAnnotator-DataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git)
library, whose types live in the `ImagesAnnotatorDataDrivers012` namespace.
`IADataImportersContext` holds an `IAnnotationsDBPtr` coming from it, which is what the
recovered records are merged into, so the dependency is mandatory: without it
the project does not configure.

### How CMake resolves it

The [cmake/enablers/template-project-data-drivers-enabler.cmake](/cmake/enablers/template-project-data-drivers-enabler.cmake)
module resolves the package with

```
find_package(${IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE} REQUIRED CONFIG)
```

It runs during the pre-`src/` enabler pass, that is before the library target
exists, so that the unit test executables declared inside the source tree can
link the imported target directly. The module also composes the imported target
name into the `IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET` variable, which by default
expands to `ImagesAnnotatorDataDrivers-0.12::ImagesAnnotatorDataDrivers-0.12`.

### The IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE cache variable

The package name is not hardcoded. It sits in the
`IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE` cache variable, whose default value is
`ImagesAnnotatorDataDrivers-0.12` - the name a stock data drivers install
carries, because that project ships with `LIB_INCLUDE_MINOR_IN_NAME=ON` just
like this one. Override it when building against an install whose name segments
were customised, for instance one configured with
`LIB_INCLUDE_MINOR_IN_NAME=OFF` or with a `LIB_NAME_SUFFIX`:

```
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DIMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE=ImagesAnnotatorDataDrivers-0
```

The imported target name follows the overridden package name automatically. The
`#include <ImagesAnnotatorDataDrivers-0.12/...>` lines of the sources do not:
they spell the header sub-directory of the default install out, so an install
named differently also needs its include directory to be reachable under that
same spelling.

### Why no version is requested

The `find_package` call deliberately asks for no version. The 0.12.0 release of
the data drivers library installs its package `Config.cmake` file without the
companion `<package>ConfigVersion.cmake`, and `find_package` only reads the
version from that companion file, so any versioned request would fail to match
a perfectly good install. The major and the minor version are already carried by
the package name itself (`...-0.12`), which is what keeps incompatible releases
apart.

### Building and installing the data drivers yourself

When no distribution package is available, build the dependency from its
sources and install it into a prefix of your choice:

```
git clone https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git
cmake -S ImagesAnnotator-DataDrivers -B ImagesAnnotator-DataDrivers/build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/iadd-install
cmake --build ImagesAnnotator-DataDrivers/build -j$(nproc)
cmake --install ImagesAnnotator-DataDrivers/build
```

Then point this project at the resulting prefix with `CMAKE_PREFIX_PATH`:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/iadd-install
cmake --build build -j$(nproc)
```

`CMAKE_PREFIX_PATH` accepts a `;` separated list, so several prefixes may be
given at once. It can be dropped entirely when the data drivers library was
installed into a prefix CMake searches by default, such as `/usr` or
`/usr/local`. The same sequence is what the
[misc/Jenkinsfile](/misc/Jenkinsfile) pipeline performs in its `Dependencies`
stage before every configure.

If the prefix is wrong or missing, the configure stops with

```
CMake Error: Could not find a package configuration file provided by
"ImagesAnnotatorDataDrivers-0.12"
```

### Why the dependency is linked PUBLIC

Unlike every other dependency of this project, the data drivers library is
linked `PUBLIC` by
[cmake/enablers/template-project-data-drivers-linker.cmake](/cmake/enablers/template-project-data-drivers-linker.cmake).
The installable header `IADataImportersContext.h` under
[src/lib/facade/public](/src/lib/facade/public) names the data drivers database
type, so a consumer of this library needs both its include directories and its
shared object - a `PRIVATE` link would hide them and break every downstream
compile.

For the same reason the generated package configuration file
[src/lib/cmake/ImportersLibraryConfig.cmake.in](/src/lib/cmake/ImportersLibraryConfig.cmake.in)
calls `find_dependency()` on the data drivers package before including the
exported targets. A downstream project therefore has to make the data drivers
prefix visible to its own configure too, exactly as described in
[Using the library in your project](/doc/sections/en_US/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md).

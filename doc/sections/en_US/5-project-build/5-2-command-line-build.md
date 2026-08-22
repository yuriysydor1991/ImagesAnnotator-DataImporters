## Command line build

The project uses the CMake build system, so the build commands are the ones
every C++ developer already knows.

Clone the repository and open its root directory in a terminal first. The only
project specific addition is `-DCMAKE_PREFIX_PATH`: the library resolves its
mandatory
[ImagesAnnotatorDataDrivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md)
with `find_package(... REQUIRED CONFIG)`, so CMake has to be told where that
library is installed.

```
# from the project root

cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix
cmake --build build -j$(nproc)
```

This creates the `build` directory (already listed in `.gitignore`), configures
the project with the CMake found in the system (see the
[Requirements](/doc/sections/en_US/3-requirements/3-requirements.md) section)
and builds every target of the project, which for a default configure means the
`libImagesAnnotatorDataImporters-0.12.so` shared library alone.

`-DCMAKE_PREFIX_PATH` may be dropped only when the data drivers library is
installed into a prefix CMake searches by default, such as `/usr` or
`/usr/local`.

A release build with the tests and a trial install into a local prefix:

```
# from the project root

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_UNIT_TESTS=ON \
  -DENABLE_COMPONENT_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

See [Installing](/doc/sections/en_US/7-installing/7-installing.md) for the
install step and
[Using the library in your project](/doc/sections/en_US/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md)
for the downstream `find_package` usage.

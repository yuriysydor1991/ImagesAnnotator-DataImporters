## IDE build

If your IDE integrates the CMake build system it is enough to open the project
root directory and press the `Build` button: no manual command search and
execution is needed.

One project specific setting has to be made first. The library requires the
[ImagesAnnotatorDataDrivers package](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md),
which is resolved with `find_package(... REQUIRED CONFIG)`, so the IDE has to
hand the install prefix of that library over to CMake. Add

```
-DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix
```

to the CMake configure arguments of the IDE profile (CLion: `Settings` ->
`Build, Execution, Deployment` -> `CMake` -> `CMake options`; VS Code with the
CMake Tools extension: the `cmake.configureArgs` setting; Qt Creator: the
`Initial CMake parameters` of the kit). Without it the configure stage stops
with a `Could not find a package configuration file provided by
"ImagesAnnotatorDataDrivers-0.11"` error.

The prefix may be omitted only when the data drivers library is already
installed into a location CMake searches by default, for example `/usr` or
`/usr/local`.

There is no application to launch: the project produces a shared library, so
the `Run` button is only useful for the test executables built with
`ENABLE_UNIT_TESTS` or `ENABLE_COMPONENT_TESTS`.

### Enabling unit testing

The project ships two independent test suites, each behind its own CMake
option: `ENABLE_UNIT_TESTS` and `ENABLE_COMPONENT_TESTS`. Both default to `OFF`
and either of them enables `enable_testing()` and makes GTest available.

To build and run the unit tests reconfigure the project with `ENABLE_UNIT_TESTS`
set to `ON` (GNU/Linux based):

```
# from the project root

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DENABLE_UNIT_TESTS=ON
cmake --build build -j$(nproc)
```

That produces the following executables:

- `UTEST_IADataImportersFacade`
- `UTEST_LibFactory`
- `UTEST_LibMain`
- `UTEST_PlainTxtFolder2DBImporter`
- `UTEST_Yolo4Folder2DBImporter`
- `UTEST_PyTorchVisionFolder2DBImporter`
- `UTEST_TypeHelper`
- `UTEST_OpenCVImageSizer`, only in a build which found OpenCV

The unit tests do not link the produced shared library. They compile the
components straight from the source tree against the gmock stand-ins kept under
`src/tests/mocks` and the per-component `src/lib/*/tests/mocks` directories, so
every component is examined in isolation.

The component tests are enabled separately and build `CTEST_DefaultLogger`,
`CTEST_LibraryRealLogger` and `CTEST_Importers`. The latter compiles nothing of the library: it links the real
shared object and drives it through the installable public headers only,
exactly the way a downstream project does.

```
# from the project root

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DENABLE_UNIT_TESTS=ON \
  -DENABLE_COMPONENT_TESTS=ON
cmake --build build -j$(nproc)
```

With both options on `ctest` reports 61 test cases, all of which are expected
to pass. See
[Run tests by the ctest](/doc/sections/en_US/6-running-the-tests/6-3-1-run-tests-by-the-ctest.md)
for the run commands.

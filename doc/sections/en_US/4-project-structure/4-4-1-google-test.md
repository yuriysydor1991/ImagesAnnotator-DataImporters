### Google Test

Both the unit and the component tests are written against GoogleTest, and the unit tests use gmock heavily: the stand-ins under `src/tests/mocks`, `src/lib/facade/tests/mocks` and `src/lib/libmain/tests/mocks` are gmock classes, so every test binary links `GTest::gtest_main` and `GTest::gmock`.

[cmake/enablers/template-project-GTest-enabler.cmake](/cmake/enablers/template-project-GTest-enabler.cmake) makes the framework available. It returns immediately unless `ENABLE_UNIT_TESTS` or `ENABLE_COMPONENT_TESTS` is on, then probes the system GoogleTest first and falls back to `FetchContent` when none is installed. Two cache variables control the fetched sources:

| Cache variable | Default |
|---|---|
| `TEMPLATE_APP_GTEST_GIT` | `https://github.com/google/googletest.git` |
| `TEMPLATE_APP_GTEST_GIT_TAG` | `v1.16.0` |

The system probe is skipped by setting `GTEST_TRY_SYSTEM_PROBE` to `OFF`, which forces the fetched copy to be used - see [Disabling system GTest probe](/doc/sections/en_US/5-project-build/testing/5-3-2-disabling-system-GTest-probe.md).

Each test directory registers its cases with `include(GoogleTest)` and `gtest_add_tests()`, so ctest sees the individual cases rather than one binary. The unit tests are compiled with `-DNDEBUG=1`, which disables the `assert()` calls of the code under test and lets a test drive an invalid argument through a function to check the value it returns.

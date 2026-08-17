### Disabling system GTest probe

By default the build system first probes the system wide installed GTest
framework and only falls back to fetching it over the Internet. To skip that
probe and always use the fetched copy set the `GTEST_TRY_SYSTEM_PROBE` CMake
variable to `OFF` (GNU/Linux based):

```
# from the project root

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DENABLE_UNIT_TESTS=ON \
  -DGTEST_TRY_SYSTEM_PROBE=OFF
cmake --build build -j$(nproc)
```

With the probe disabled the build system makes GTest available through the
Internet for the current project only, at the version pinned in the
[cmake/enablers/template-project-GTest-enabler.cmake](/cmake/enablers/template-project-GTest-enabler.cmake)
file. The repository and the tag are held in the `TEMPLATE_APP_GTEST_GIT` and
`TEMPLATE_APP_GTEST_GIT_TAG` cache variables, so a different GTest revision can
be requested without editing the file:

```
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DENABLE_UNIT_TESTS=ON \
  -DGTEST_TRY_SYSTEM_PROBE=OFF \
  -DTEMPLATE_APP_GTEST_GIT_TAG=v1.15.2
```

The variable is only consulted when at least one of `ENABLE_UNIT_TESTS` and
`ENABLE_COMPONENT_TESTS` is `ON`; otherwise the GTest enabler returns
immediately and no test framework is looked up at all.

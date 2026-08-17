## Optional for the tests

If necessary to build and execute the available tests install GTest with GMock, or ensure available Internet access in order to let the project make them available by itself (GNU/Linux based):

```
sudo apt install -y libgtest-dev libgmock-dev
```

GMock is used by the unit tests, which compile the library sources straight from the tree against the stand-ins under `src/tests/mocks` and `src/lib/*/tests/mocks`. The component tests link the real shared library instead and drive it through the public headers only, the same way a downstream project does.

The tests are built only when requested: `ENABLE_UNIT_TESTS=ON` for the unit ones and `ENABLE_COMPONENT_TESTS=ON` for the component ones. Examine the [Enabling unit testing](/doc/sections/en_US/5-project-build/testing/5-3-1-enabling-unit-testing.md) and the [Disabling system GTest probe](/doc/sections/en_US/5-project-build/testing/5-3-2-disabling-system-GTest-probe.md) subsections, and the [Project tests](/doc/sections/en_US/4-project-structure/4-4-project-tests.md) section for more info.

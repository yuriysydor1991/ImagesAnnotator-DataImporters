# Requirements

This section contains the list of the required packages and tools that must be present in the system in order to build the importers library.

One dependency is mandatory - the CMake configuration step fails without it:

- The **ImagesAnnotatorDataDrivers** library. The importers build their image records and fill their annotations database through it, so it is resolved with `find_package(ImagesAnnotatorDataDrivers-0.12 REQUIRED CONFIG)` and linked `PUBLIC`, because the installable headers name its database type. Build and install it from [https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) first and point this project at its install prefix with `-DCMAKE_PREFIX_PATH=<prefix>`. Examine the [The data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md) subsection.

Everything listed below is either a basic build tool or an optional addition needed for a particular development task only.

1. [Required tools for the GNU/Linux based OS](/doc/sections/en_US/3-requirements/3-1-required-tools-for-the-GNU-Linux-based-OS.md)
1. [Required tools for the MS Windows based OS](/doc/sections/en_US/3-requirements/3-2-required-tools-for-the-MS-Windows-based-OS.md)
1. [Optional for the tests](/doc/sections/en_US/3-requirements/3-3-optional-for-the-tests.md)
1. [Optional for the documentation](/doc/sections/en_US/3-requirements/3-4-optional-for-the-documentation.md)
1. [Optional for the code formatting](/doc/sections/en_US/3-requirements/3-5-optional-for-the-code-formatting.md)
1. [Optional for the code analyzer (cppcheck)](/doc/sections/en_US/3-requirements/3-6-optional-for-the-code-analyzer-cppcheck.md)
1. [Optional for the code analyzer with clang-tidy](/doc/sections/en_US/3-requirements/3-7-optional-for-the-code-analyzer-with-clang-tidy.md)

The ImagesAnnotatorDataDrivers library is the only mandatory third party dependency the library has. OpenCV is the one optional dependency: with it the library also ships an image size reader of its own, and without it the configure only says so - see [Enabling the OpenCV image size reader](/doc/sections/en_US/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md). Everything else listed above is optional tooling.

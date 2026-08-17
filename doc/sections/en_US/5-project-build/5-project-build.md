# Project build

The project is a CMake library project: it produces the shared object
`libImagesAnnotatorDataImporters-0.11.so` together with its installable public
headers and its CMake package. No application binary is built, only the library
and its test executables.

Before the first configure make sure the mandatory
[ImagesAnnotatorDataDrivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md)
is installed and reachable: every configure command in the sections below needs
a `-DCMAKE_PREFIX_PATH` pointing at its install prefix.

1. [IDE build](/doc/sections/en_US/5-project-build/5-1-IDE-build.md)
1. [Command line build](/doc/sections/en_US/5-project-build/5-2-command-line-build.md)
1. [Quick build scripts](/doc/sections/en_US/5-project-build/5-38-quick-build-scripts.md)
1. [The ImagesAnnotatorDataDrivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md)
1. Enabling testing
    1. [Enabling unit testing](/doc/sections/en_US/5-project-build/testing/5-3-1-enabling-unit-testing.md)
    1. [Disabling system GTest probe](/doc/sections/en_US/5-project-build/testing/5-3-2-disabling-system-GTest-probe.md)
1. [Documentation build](/doc/sections/en_US/5-project-build/documentation/5-4-documentation-build.md)
1. [Configuring the documentation install support](/doc/sections/en_US/5-project-build/documentation/5-5-configuring-the-documentation-install-support.md)
1. [Customizing the installable library name segments](/doc/sections/en_US/5-project-build/5-23-customizing-library-name-segments.md)
1. Code quality & sanitizers
    1. [Enabling and performing code formatting target](/doc/sections/en_US/5-project-build/code-quality/5-6-enabling-and-performing-code-formatting-target.md)
    1. [Enabling the static code analyzer target with cppcheck](/doc/sections/en_US/5-project-build/code-quality/5-7-enabling-the-static-code-analyzer-target-with-cppcheck.md)
    1. [Enabling the static code analyzer with clang-tidy](/doc/sections/en_US/5-project-build/code-quality/5-8-enabling-static-code-analyzer-with-clang-tidy.md)
1. Containers / CI
    1. [Enabling Jenkins pipeline inside Docker container](/doc/sections/en_US/5-project-build/containers-ci/5-17-enabling-Jenkins-pipeline-inside-Docker-container.md)
1. Packagers
    1. [Enabling DEB package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-10-enabling-DEB-package-generation-with-cpack.md)
    1. [Enabling FreeBSD pkg package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-20-enabling-FreeBSD-pkg-package-generation-with-cpack.md)
    1. [Enabling WIX MSI package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-21-enabling-WIX-MSI-package-generation-with-cpack.md)
    1. [Enabling RPM package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-22-enabling-RPM-package-generation-with-cpack.md)
1. Libraries
    1. [Enabling the OpenCV image size reader (optional)](/doc/sections/en_US/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md)

The ImagesAnnotatorDataDrivers library is the only mandatory third party
dependency this library has. OpenCV is optional: without it the library simply
ships no image size reader of its own.

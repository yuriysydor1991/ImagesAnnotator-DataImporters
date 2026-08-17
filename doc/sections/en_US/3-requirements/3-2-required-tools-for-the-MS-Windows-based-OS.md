## Required tools for the MS Windows based OS

In order to build the library binaries download the install tool for the developer tools from the MSVC download page [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/) and search and enable the Native C++ application development section.

Separately each project may be downloaded and installed from different sources (**may require additional tools download and install**):
- The Git versioning system at [https://git-scm.com/downloads/win](https://git-scm.com/downloads/win)
- The CMake build system at [https://cmake.org/download/](https://cmake.org/download/)
- The Windows 10 SDK at [https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)

The one mandatory dependency applies here as well. Build and install the ImagesAnnotatorDataDrivers library from [https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) first and hand its install prefix over with `-DCMAKE_PREFIX_PATH=<prefix>`, as described in the [The data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md) subsection.

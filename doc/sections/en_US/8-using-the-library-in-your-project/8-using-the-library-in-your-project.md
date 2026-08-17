# Using the library in your project

The library is meant to be consumed by other projects: it produces no executable of its own, only the `libImagesAnnotatorDataImporters-0.11.so` shared object with its installable headers and a CMake package. This section describes what a downstream project has to do to build against it.

## What has to be installed first

Two things have to be installed before a consumer may be configured:

- the [ImagesAnnotatorDataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) library, which defines the annotations database and the image records this library fills - see the [The data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md) subsection;
- this library itself, built and installed as described in the [Installing](/doc/sections/en_US/7-installing/7-installing.md) section.

The install puts the following into the chosen prefix:

```
<prefix>/include/ImagesAnnotatorDataImporters-0.11/     the public headers
<prefix>/lib/libImagesAnnotatorDataImporters-0.11.so    the shared object, soname .so.0
<prefix>/lib/cmake/ImagesAnnotatorDataImporters-0.11/   the CMake package files
```

## Finding the package with CMake

```cmake
cmake_minimum_required(VERSION 3.13)

project(MyImportingTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ImagesAnnotatorDataImporters-0.11 0.11 REQUIRED)

add_executable(my-importing-tool main.cpp)

target_link_libraries(
  my-importing-tool
  PRIVATE ImagesAnnotatorDataImporters-0.11::ImagesAnnotatorDataImporters-0.11
)
```

Both the package name and the imported target carry the library major and minor version, so a future release may be installed side by side with this one. The version request is matched against the installed `ImagesAnnotatorDataImporters-0.11ConfigVersion.cmake` file with the `SameMajorVersion` compatibility rule.

The installed package configuration file, generated from [src/lib/cmake/ImportersLibraryConfig.cmake.in](/src/lib/cmake/ImportersLibraryConfig.cmake.in), calls `find_dependency()` on the data drivers package before it reads the exported targets. The data drivers library is linked `PUBLIC` because the installable headers of this library name its database and record types, so linking the target above brings the data drivers include path and shared object along with it. A separate `find_package()` for the data drivers is not needed in a consumer, although calling one does no harm.

The C++ `17` standard is not optional: the public headers and the records they name are compiled as C++17.

## Pointing CMake at the install prefixes

When either library was installed outside of the system default prefixes, name their prefixes through `CMAKE_PREFIX_PATH` when configuring your project:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH="/opt/iadd;/opt/iadi"
```

The very same variable is what this library itself needs at its own configure time to find the data drivers.

## Including the headers

Two include roots are exported, so both spellings compile:

```cpp
#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>  // recommended
#include <LibraryFacade.h>                                    // also works
```

Prefer the prefixed one. Header names such as `LibraryFacade.h`, `LibraryContext.h` and `ILib.h` are generic enough to collide in a busy include path - the data drivers library installs headers of exactly those names, and with both plain include roots in play the short spelling picks whichever of the two the compiler happens to see first.

`LibraryFacade.h` includes every other public header of the library, so it is usually the only one a consumer names.

## Aliasing the namespaces

The interface namespaces of both libraries carry their major and minor version numbers. Alias them once and the version bump stays a one line change:

```cpp
namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters011;
```

## What your project has to supply

- **The database.** `LibraryContext::set_db()` takes an `ImagesAnnotatorDataDrivers011::IAnnotationsDBPtr` - the destination the recovered records are merged into. An empty one comes from `iadd::LibraryFacade::create_annotations_db()`; hand over the one of `iadd::LibraryFacade::open_annotations_db("project.json")` instead to import **into** an existing project, and the import appends the images that project does not hold yet. Either way the database is what stores the result, through its own `store_db()`.
- **The source directory.** `LibraryContext::set_import_path()` has to name an existing directory holding the dataset of the layout the context stands for. Nothing inside it is written to: an import is a read only pass.
- **An image size reader, for two of the three layouts.** The YOLO v4 layout stores its boxes divided by the size of their image and the PyTorch Vision one stores an annotation as the cropped image itself, so neither can be read back without measuring the pictures - and the library decodes no image format itself. Implement `IImageSizeFacility` over the imaging stack your project already links and pass the instance in through `LibraryContext::set_image_sizer()`. A library built with OpenCV ships a reader of its own and fills an empty slot with it, so this is only mandatory for a consumer of a build without OpenCV, or for one that wants its own measuring. The [The dataset importers API](/doc/sections/en_US/4-project-structure/4-9-the-dataset-importers-api.md) subsection carries an implementation sketch, and [Enabling the OpenCV image size reader](/doc/sections/en_US/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md) covers the built-in one. The plain text layout needs no reader at all.

## A minimal consumer

The `main.cpp` below reads a plain text dataset back and stores it as a project file, through the single shot `ILib::perform_import` entry point:

```cpp
#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <ImagesAnnotatorDataImporters-0.11/LibraryFacade.h>

#include <iostream>
#include <memory>

namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters011;

int main()
{
  auto db = iadd::LibraryFacade::create_annotations_db();

  if (db == nullptr) {
    std::cerr << "fail to create the annotations database\n";
    return 1;
  }

  auto ctx = iadi::LibraryFacade::create_plain_txt_library_context();

  ctx->set_import_path("plain-dataset");
  ctx->set_db(db);

  auto lib = iadi::LibraryFacade::create_library(ctx);

  if (lib == nullptr || !lib->perform_import(ctx)) {
    std::cerr << "the import has failed\n";
    return 1;
  }

  if (!db->store_db("project.json")) {
    std::cerr << "fail to store the project file\n";
    return 1;
  }

  std::cout << "imported " << ctx->get_imported_records()
            << " image records with the library version "
            << iadi::LibraryFacade::library_version() << '\n';

  return 0;
}
```

Instantiate `iadi::Yolo4ImportLibraryContext` or `iadi::PyTorchImportLibraryContext` instead to get one of the two other layouts, described in the [The read dataset layouts](/doc/sections/en_US/4-project-structure/4-10-the-read-dataset-layouts.md) subsection - both of them need the image size reader above. Building the importer directly with `iadi::LibraryFacade::create_importer()` gives the same result with a finer grained control - see the [The dataset importers API](/doc/sections/en_US/4-project-structure/4-9-the-dataset-importers-api.md) subsection.

## Running the result

Both shared objects have to be reachable by the dynamic linker at the run time. When they were installed into a prefix the system does not search by default, either register that prefix (for example by adding it to `/etc/ld.so.conf.d/` and running `ldconfig`) or name it in the environment:

```
LD_LIBRARY_PATH=/opt/iadi/lib:/opt/iadd/lib ./my-importing-tool
```

`iadi::LibraryFacade::library_version()` reports the version of the binary that was actually loaded, which is the quickest way to tell which of several installed copies your program ended up with.

## A worked example inside this project

The `CTEST_Importers` component test, [src/importers/tests/component/Importers/CTEST_Importers.cpp](/src/importers/tests/component/Importers/CTEST_Importers.cpp), links the produced shared library and drives it through the installable headers only, exactly the way a downstream project does - including a small `IImageSizeFacility` implementation. Enable it with the `ENABLE_COMPONENT_TESTS` CMake option described in the [Project build](/doc/sections/en_US/5-project-build/5-project-build.md) section.

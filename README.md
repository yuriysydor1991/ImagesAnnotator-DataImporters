**The ImagesAnnotator annotations dataset importers library**

# What is it

The `ImagesAnnotatorDataImporters` is a C++17 shared library which reads the training dataset layouts the machine learning frameworks use back into the annotations of the [ImagesAnnotator](https://github.com/yuriysydor1991/ImagesAnnotator.git) application - the annotated images with the named rectangles drawn over them.

It is the exact counterpart of the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library: what that one writes, this one reads. A dataset that was exported may be brought back into a project, a dataset produced by some other tool may be brought into one for the first time, and the ImagesAnnotator application together with **any other tool** shares a single implementation of that reading.

Everything a consuming project touches is hidden behind the abstract interfaces of the [src/lib/facade/public](/src/lib/facade/public) headers, so no implementation class, and none of the library third party dependencies, leak into the downstream code.

See more at the [kytok.org.ua](http://www.kytok.org.ua/)

💵 Donate at [http://kytok.org.ua/page/pozertvy](http://kytok.org.ua/page/pozertvy)

# Features

- **Nine dataset layouts into one database** - selected by the `IADataImportersContext` descendant instantiated and implemented by one importer class each, the very nine the sibling exporters library writes:
  - **Plain text** (`PlainTxtImportContext`) - one `<annotation-name>.txt` file per annotation name, each line naming an image and its rectangles in the pixels of that image;
  - **YOLO v4 (darknet)** (`Yolo4ImportContext`) - the whole darknet training directory of the YOLO v4 detector: the class names of `data/obj.names`, the image list of `data/train.txt` and the normalised `.txt` label file of every image, with the `data/obj.data` descriptor followed when it is there, so that a directory laid out by some other tool is read the way that tool named it;
  - **Ultralytics YOLO - detection, oriented bounding boxes and segmentation** (`UltralyticsDetectImportContext`, `UltralyticsObbImportContext` and `UltralyticsSegmentImportContext`) - the layout every Ultralytics release trains from: the `data.yaml` descriptor naming the classes and the image directories, and one label file per image under `labels/` beside them. The three differ in that label line alone - the box, its four corners or the polygon outlining the object - and the last two come back as the rectangle which holds what they draw;
  - **COCO JSON** (`CocoImportContext`) - the COCO object detection dataset: the single JSON descriptor naming the pictures, their size, the categories and the `[x, y, width, height]` box of every annotation, which are the very four numbers a rectangle of the database holds;
  - **Pascal VOC XML** (`PascalVocImportContext`) - the Pascal VOC devkit shape: one XML descriptor per image under `Annotations/`, or lying beside the pictures themselves, which is how [LabelImg](https://github.com/HumanSignal/labelImg) saves its own work;
  - **Create ML JSON** (`CreateMLImportContext`) - the Create ML object detection dataset: the flat directory of the pictures and the one JSON descriptor beside them, whose boxes are written centred and come back on the corner they were drawn from;
  - **PyTorch Vision `ImageFolder`** (`PyTorchImportContext`) - the classification layout the PyTorch Vision `ImageFolder` dataset reads: one directory per annotation name holding the cropped images, each of which comes back as an image record whose single rectangle covers it whole.
- **A one shot entry point** - fill the `IADataImportersContext` descendant of the wanted layout with the source directory and the destination database, and `IADataImportersLib::perform_import()` builds the right importer and runs it. `IADataImportersFacade::create_importer()` gives the same result with a finer grained control.
- **Nothing is overwritten** - an import is a read only pass over its directory. The recovered records point at the image files where they already lie, and they are merged into the database through `IAnnotationsDB::add_images_db()`, which keeps an image the database already holds. So a dataset may be imported into a project being edited, and importing the same dataset twice adds its images once.
- **Robust over a partial dataset** - a malformed line, an image file the dataset names but does not hold, a label naming an unknown class, a picture that cannot be measured: each is logged and skipped, the import run itself carries on.
- **No image codec of its own** - the layouts which do not store their rectangles in the pixels of their image ask the consuming project to measure the pictures through the `IImageSizeFacility` interface, over whatever imaging stack that project already links. A build which found OpenCV ships such a reader itself, so a consumer with no imaging stack still gets those imports. The plain text, the COCO, the Pascal VOC and the Create ML layouts need no measurement at all.
- **No JSON or XML dependency either** - the two descriptor formats are read by the library own document readers under [src/parsers](/src/parsers), exactly as the sibling exporters library writes those very descriptors without one. Nothing is linked for them, and nothing of either reader reaches the installed interface.
- **A versioned installable interface** - the namespace, the binary, the header directory and the CMake package all carry the `0.14` major and minor pair, so two minor releases install side by side.

# Usage example

The library is consumed through a CMake package. The following program was compiled, linked and run against the installed library:

```cmake
cmake_minimum_required(VERSION 3.13)
project(MyTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ImagesAnnotatorDataImporters-0.14 REQUIRED)

add_executable(mytool main.cpp)
target_link_libraries(mytool ImagesAnnotatorDataImporters-0.14::ImagesAnnotatorDataImporters-0.14)
```

```cpp
#include <ImagesAnnotatorDataDrivers-0.12/IADataDriversFacade.h>
#include <ImagesAnnotatorDataImporters-0.14/IADataImportersFacade.h>

#include <iostream>
#include <memory>

namespace iadd = ImagesAnnotatorDataDrivers012;
namespace iadi = ImagesAnnotatorDataImporters014;

int main(int argc, char** argv)
{
  if (argc < 3) { return 1; }

  auto db = iadd::IADataDriversFacade::create_annotations_db();

  if (db == nullptr) { return 1; }

  auto ctx = iadi::IADataImportersFacade::create_yolo4_library_context();

  ctx->set_import_path(argv[1]);
  ctx->set_db(db);

  auto lib = iadi::IADataImportersFacade::create_default_lib();

  if (lib == nullptr || !lib->perform_import(ctx)) {
    std::cerr << "the import has failed\n";
    return 1;
  }

  if (!db->store_db(argv[2])) {
    std::cerr << "fail to store the project file\n";
    return 1;
  }

  std::cout << "imported " << ctx->get_imported_records() << " records with "
            << iadi::IADataImportersFacade::library_version() << "\n";

  return 0;
}
```

The `ImagesAnnotatorDataImporters014` namespace name carries the library major and minor version numbers on purpose: two library versions may coexist inside a single translation unit without any symbol clash. Alias it, as shown above, and the version bump stays a one line change on your side.

Both `#include <ImagesAnnotatorDataImporters-0.14/IADataImportersFacade.h>` and a plain `#include <IADataImportersFacade.h>` work for an installed consumer, since the library exports the include root along with its versioned subdirectory. The prefixed form is the recommended one: `IImporter.h` and `ImportersAPI.h` are generic enough to collide in a busy include path, and the prefix pins the library version a translation unit is talking to.

The program above reads the YOLO v4 layout, whose rectangles are stored divided by the size of their image, so it needs a library built with OpenCV. Give an `IImageSizeFacility` of your own to `ctx->set_image_sizer()` otherwise, or start from the plain text layout, which needs none.

More on the API and on the read datasets is in the [dataset importers API](/doc/sections/en_US/4-project-structure/4-9-the-dataset-importers-api.md), the [read dataset layouts](/doc/sections/en_US/4-project-structure/4-10-the-read-dataset-layouts.md) and the [using the library in your project](/doc/sections/en_US/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md) documentation sections.

# Dependencies

| CMake option | Library | Why it is needed |
| --- | --- | --- |
| (always on) | [ImagesAnnotatorDataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) | it defines the annotations database the importers fill and the image records they build |
| `ENABLE_OPENCV` | [OpenCV](https://opencv.org/) | optional: with it the library ships an image size reader of its own, so that a consumer with no imaging stack still gets the layouts which need one |

The data drivers library is **mandatory**. It has to be installed beforehand, it is resolved with `find_package(ImagesAnnotatorDataDrivers-0.12 REQUIRED CONFIG)` and it is linked **publicly**, because the installable headers of this library name its database type. Point the configure at its install prefix with `-DCMAKE_PREFIX_PATH=<prefix>` when it does not sit in a system default one. The [data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md) section covers it in full, the package name included.

OpenCV is **optional** and `ENABLE_OPENCV=ON` means *probe*, not *require*: a system without it configures and builds just the same, only without the built-in image size reader. It is linked **privately**: no public header exposes an OpenCV type, so a consuming project needs no OpenCV of its own. See the [enabling the OpenCV image size reader](/doc/sections/en_US/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md) section.

The logging component is compiled straight into the shared library, so no logger implementation has to be supplied by the consumer.

# Build and test

The plain build, against a data drivers install in `$HOME/iadd-install`:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/iadd-install
cmake --build build -j$(nproc)
```

The tests are off by default. To build and run them:

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/iadd-install \
  -DENABLE_UNIT_TESTS=ON -DENABLE_COMPONENT_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

With both options on the suite holds 90 test cases. The `ENABLE_UNIT_TESTS` targets are compiled straight from the sources against the gmock stand-ins of [src/tests/mocks](/src/tests/mocks), while the `ENABLE_COMPONENT_TESTS` `CTEST_Importers` links the real shared library and drives it through the public headers only - exactly the way a downstream project does.

Installing is a usual `sudo cmake --install build`, described in detail in the [installing](/doc/sections/en_US/7-installing/7-installing.md) section.

# Where the code came from

The repository started life as the `lib` branch of the [cpp-app-template](https://github.com/yuriysydor1991/cpp-app-template) project and was filled with the importers written against the layouts the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library writes, so that the two halves of the pair round trip. The [ImagesAnnotator](https://github.com/yuriysydor1991/ImagesAnnotator.git) application is to consume this library rather than to grow an import of its own.

The annotations database itself - the project file parser, the serializer and the merging rules - is **not** a part of this library. It lives in the sibling [ImagesAnnotator-DataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) project, which this one builds its records through.

Examine the `doc` directory for possible translations of a current md document:
- `uk_UA` at [doc/README.uk_UA.md](doc/README.uk_UA.md)

# Documentation contents

**Document is under the refinement**

1. [Requirements](/doc/sections/en_US/3-requirements/3-requirements.md)
    1. [Required tools for the GNU/Linux based OS](/doc/sections/en_US/3-requirements/3-1-required-tools-for-the-GNU-Linux-based-OS.md)
    1. [Required tools for the MS Windows based OS](/doc/sections/en_US/3-requirements/3-2-required-tools-for-the-MS-Windows-based-OS.md)
    1. [Optional for the tests](/doc/sections/en_US/3-requirements/3-3-optional-for-the-tests.md)
    1. [Optional for the documentation](/doc/sections/en_US/3-requirements/3-4-optional-for-the-documentation.md)
    1. [Optional for the code formatting](/doc/sections/en_US/3-requirements/3-5-optional-for-the-code-formatting.md)
    1. [Optional for the code analyzer (cppcheck)](/doc/sections/en_US/3-requirements/3-6-optional-for-the-code-analyzer-cppcheck.md)
    1. [Optional for the code analyzer with clang-tidy](/doc/sections/en_US/3-requirements/3-7-optional-for-the-code-analyzer-with-clang-tidy.md)
1. [Project structure](/doc/sections/en_US/4-project-structure/4-project-structure.md)
    1. [Project diagrams](/doc/sections/en_US/4-project-structure/4-0-project-diagrams.md)
    1. [Where the importers implementation lives](/doc/sections/en_US/4-project-structure/4-1-implement-code-straight-away.md)
    1. [The library's installable include header files](/doc/sections/en_US/4-project-structure/4-8-the-librarys-installable-include-header-files.md)
    1. [The dataset importers API](/doc/sections/en_US/4-project-structure/4-9-the-dataset-importers-api.md)
    1. [The read dataset layouts](/doc/sections/en_US/4-project-structure/4-10-the-read-dataset-layouts.md)
    1. [Version tracking and other project parameters](/doc/sections/en_US/4-project-structure/4-3-version-tracking-and-other-project-parameters.md)
    1. [Project tests](/doc/sections/en_US/4-project-structure/4-4-project-tests.md)
        1. [Google Test](/doc/sections/en_US/4-project-structure/4-4-1-google-test.md)
1. [Project build](/doc/sections/en_US/5-project-build/5-project-build.md)
    1. [IDE build](/doc/sections/en_US/5-project-build/5-1-IDE-build.md)
    1. [Command line build](/doc/sections/en_US/5-project-build/5-2-command-line-build.md)
    1. [Quick build scripts](/doc/sections/en_US/5-project-build/5-38-quick-build-scripts.md)
    1. [The data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md)
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
    1. Containers & CI
        1. [Enabling Jenkins pipeline inside Docker container](/doc/sections/en_US/5-project-build/containers-ci/5-17-enabling-Jenkins-pipeline-inside-Docker-container.md)
    1. Packagers
        1. [Enabling DEB package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-10-enabling-DEB-package-generation-with-cpack.md)
        1. [Enabling FreeBSD pkg package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-20-enabling-FreeBSD-pkg-package-generation-with-cpack.md)
        1. [Enabling WIX MSI package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-21-enabling-WIX-MSI-package-generation-with-cpack.md)
        1. [Enabling RPM package generation with cpack](/doc/sections/en_US/5-project-build/packagers/5-22-enabling-RPM-package-generation-with-cpack.md)
    1. Libraries
        1. [Enabling the OpenCV image size reader](/doc/sections/en_US/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md)
1. Running the tests
    1. [Run tests by the ctest](/doc/sections/en_US/6-running-the-tests/6-3-1-run-tests-by-the-ctest.md)
    1. [Manual tests run](/doc/sections/en_US/6-running-the-tests/6-3-2-manual-tests-run.md)
1. [Installing](/doc/sections/en_US/7-installing/7-installing.md)
1. [Using the library in your project](/doc/sections/en_US/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md)

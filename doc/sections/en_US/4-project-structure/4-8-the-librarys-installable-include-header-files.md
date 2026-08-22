## The library's installable include header files

The [src/lib/facade/public](/src/lib/facade/public) directory holds the whole installable interface of the library - fifteen headers, all in the `ImagesAnnotatorDataImporters013` namespace. Six of them are the API proper and sit at the root of the directory:

| Header | Declares |
|---|---|
| [LibraryFacade.h](/src/lib/facade/public/LibraryFacade.h) | the `LibraryFacade` static methods, the entry point of the library |
| [ILib.h](/src/lib/facade/public/ILib.h) | `ILib::perform_import()` - runs an import described by a `LibraryContext` |
| [LibraryContext.h](/src/lib/facade/public/LibraryContext.h) | the data accessors of a `perform_import()` and of an `import_db()`, plus the `get_importer()` and `get_imported_records()` out-ones |
| [IImporter.h](/src/lib/facade/public/IImporter.h) | `IImporter::import_db()` - a single importer used on its own |
| [IImageSizeFacility.h](/src/lib/facade/public/IImageSizeFacility.h) | the interface the consuming project implements to measure images |
| [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h) | the `IADI_API` visibility macro |

The nine remaining ones are the layout specific `LibraryContext` descendants - one per dataset layout the library reads - and they sit together in the [contexts](/src/lib/facade/public/contexts) subdirectory instead of beside the six above, which is the layout the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library carries for its own nine:

| Header | Declares |
|---|---|
| [contexts/PlainTxtImportLibraryContext.h](/src/lib/facade/public/contexts/PlainTxtImportLibraryContext.h) | the `LibraryContext` of the plain text dataset layout |
| [contexts/Yolo4ImportLibraryContext.h](/src/lib/facade/public/contexts/Yolo4ImportLibraryContext.h) | the `LibraryContext` of the YOLO v4 (darknet) dataset layout |
| [contexts/UltralyticsDetectImportLibraryContext.h](/src/lib/facade/public/contexts/UltralyticsDetectImportLibraryContext.h) | the `LibraryContext` of the Ultralytics YOLO detection dataset layout |
| [contexts/UltralyticsObbImportLibraryContext.h](/src/lib/facade/public/contexts/UltralyticsObbImportLibraryContext.h) | the `LibraryContext` of the Ultralytics YOLO oriented bounding box dataset layout |
| [contexts/UltralyticsSegmentImportLibraryContext.h](/src/lib/facade/public/contexts/UltralyticsSegmentImportLibraryContext.h) | the `LibraryContext` of the Ultralytics YOLO instance segmentation dataset layout |
| [contexts/CocoImportLibraryContext.h](/src/lib/facade/public/contexts/CocoImportLibraryContext.h) | the `LibraryContext` of the COCO object detection dataset layout |
| [contexts/PascalVocImportLibraryContext.h](/src/lib/facade/public/contexts/PascalVocImportLibraryContext.h) | the `LibraryContext` of the Pascal VOC dataset layout |
| [contexts/CreateMLImportLibraryContext.h](/src/lib/facade/public/contexts/CreateMLImportLibraryContext.h) | the `LibraryContext` of the Create ML object detection dataset layout |
| [contexts/PyTorchImportLibraryContext.h](/src/lib/facade/public/contexts/PyTorchImportLibraryContext.h) | the `LibraryContext` of the PyTorch Vision dataset layout |

Each of the nine reaches the six above through a `../` include, which is what keeps them resolving once installed: the include root of a consumer is the directory holding `ImagesAnnotatorDataImporters-0.13/`, so a plain `#include "LibraryContext.h"` from within `contexts/` would look for it at that root and miss. None of that concerns a consumer, which reaches all nine through `LibraryFacade.h` - it includes them - and only a project spelling a layout header out directly names the `contexts/` component itself.

[src/lib/facade/CMakeLists.txt](/src/lib/facade/CMakeLists.txt) installs the directory as a whole under `include/${PROJECT_LIBRARY_NAME}`, which for the current name and version is `include/ImagesAnnotatorDataImporters-0.13/`. Both that sub-directory and the plain include root are exported by the library target, so a consumer may write either form:

```cpp
#include <ImagesAnnotatorDataImporters-0.13/LibraryFacade.h>  // recommended
#include <LibraryFacade.h>                                    // also works
```

The prefixed form is the safe one - names such as `IImporter.h` are generic enough to collide in a busy include path, and both the data drivers and the exporters libraries of this family install a `LibraryFacade.h` of their own.

### Why only these headers are visible

The library is built with `CXX_VISIBILITY_PRESET hidden`, so only the entities marked with `IADI_API` leave the shared object. That is a correctness requirement rather than a size optimisation. The ImagesAnnotatorDataDrivers library this one links against comes from the same project template and exports its own `default_logger::DefaultLogger` and `project_decls` symbols. Were both sets exported, the dynamic linker would bind one library's calls to the other library's definitions.

Visibility alone leaves one hole. A `std::make_shared` instantiation names its class in its own mangled name and stays weak and exported whatever the visibility is, so the implementation namespace here is `iadi0impl` and not the `lib0impl` the project template - and the data drivers library with it - uses. The two `LibFactory` classes do not even share a vtable layout, and the linker would otherwise be free to bind one library's `std::make_shared<lib0impl::LibFactory>()` to the other's definition.

So a new public class belongs in [src/lib/facade/public](/src/lib/facade/public) - a new dataset layout context in its [contexts](/src/lib/facade/public/contexts) subdirectory, reaching the headers above it through `../` - and has to be marked with `IADI_API`; every other component under [src](/src) stays private to the shared object and is reached through the abstract interfaces above.

### The installed CMake package

Along with the binary and the headers the build installs a CMake package, generated by [src/lib/cmake/lib-cmake-module-gen.cmake](/src/lib/cmake/lib-cmake-module-gen.cmake) from [src/lib/cmake/ImportersLibraryConfig.cmake.in](/src/lib/cmake/ImportersLibraryConfig.cmake.in). It lands in `<libdir>/cmake/ImagesAnnotatorDataImporters-0.13/` and consists of three files: the exported targets, an `ImagesAnnotatorDataImporters-0.13ConfigVersion.cmake` written by `write_basic_package_version_file()` with `SameMajorVersion` compatibility, and the `ImagesAnnotatorDataImporters-0.13Config.cmake` that `find_dependency()`s the data drivers package before including the targets - the public headers name its database and record types, so it has to be resolved first.

A downstream project therefore needs no more than:

```cmake
find_package(ImagesAnnotatorDataImporters-0.13 0.13 REQUIRED)

target_link_libraries(
  your_target
  PRIVATE ImagesAnnotatorDataImporters-0.13::ImagesAnnotatorDataImporters-0.13
)
```

See [Installing](/doc/sections/en_US/7-installing/7-installing.md) and [Using the library in your project](/doc/sections/en_US/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md).

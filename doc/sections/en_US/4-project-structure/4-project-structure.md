# Project structure

The repository builds a single shared library - `libImagesAnnotatorDataImporters-0.14.so` - that turns a training dataset on disk into an ImagesAnnotator annotations database. There is no executable: apart from the library itself the build only produces test binaries.

The library is the reverse of the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) one, so that a dataset produced by any tool - by that exporters library above all - may be read back into an annotations project, and so that the [ImagesAnnotator](https://github.com/yuriysydor1991/ImagesAnnotator.git) application and any other tool share one implementation of that reading. The annotation records themselves are not defined here: they are built through the [ImagesAnnotator-DataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) library, which had earlier taken the annotations database out of that same application.

The top level layout:

| Path | Contents |
|---|---|
| [src/lib/facade/public](/src/lib/facade/public) | the installable public headers - the whole `ImagesAnnotatorDataImporters014` interface |
| [src/lib/facade/IADataImportersFacade.cpp](/src/lib/facade/IADataImportersFacade.cpp) | the facade implementation, the entry point of the library |
| [src/lib/libmain](/src/lib/libmain) | `LibMain.cpp` and `LibFactory.cpp` - the implementation core |
| [src/lib/cmake](/src/lib/cmake) | generation of the installable CMake package of the library |
| [src/importers](/src/importers) | one sub-directory per dataset layout family - `PlainTxt`, `Yolo4`, `Ultralytics`, `Coco`, `PascalVoc`, `CreateML` and `PyTorch` - each carrying its own `IImporter` implementations, its `CMakeLists.txt` and its unit tests, plus the `ImportersAliases.h`, the `Folder2DBImporter` base they all share and the `JsonDescriptor2DBImporter` the two JSON described ones share |
| [src/parsers](/src/parsers) | the JSON and the XML document readers the descriptor layouts are read through, one sub-component each, tested on their own documents |
| [src/sizers](/src/sizers) | `create_builtin_image_sizer()` and the optional OpenCV `IImageSizeFacility` behind it |
| [src/helpers](/src/helpers) | `IHelper.h`, `TypeHelper.h`, `StringHelper.h` and `Utf8Helper.h`, all header only |
| [src/log](/src/log) | the logging macros and the simple logger compiled into the library |
| [cmake](/cmake) | the build system: compile options and the dependency enablers |
| [doc](/doc) | this documentation and the project diagrams |
| [misc](/misc) | the Jenkins pipeline and the Docker files |

The implementation lives in the `iadi0impl`, `iannotator::importers`, `iannotator::importers::helpers`, `iannotator::importers::parsers` and `iannotator::importers::sizers` namespaces. None of them is installed - a consuming project only ever sees `ImagesAnnotatorDataImporters014` (an `iadi` alias is recommended).

1. [Project diagrams](/doc/sections/en_US/4-project-structure/4-0-project-diagrams.md)
1. [Where the importers implementation lives](/doc/sections/en_US/4-project-structure/4-1-implement-code-straight-away.md)
1. [The dataset importers API](/doc/sections/en_US/4-project-structure/4-9-the-dataset-importers-api.md)
1. [The read dataset layouts](/doc/sections/en_US/4-project-structure/4-10-the-read-dataset-layouts.md)
1. [The library's installable include header files](/doc/sections/en_US/4-project-structure/4-8-the-librarys-installable-include-header-files.md)
1. [Version tracking and other project parameters](/doc/sections/en_US/4-project-structure/4-3-version-tracking-and-other-project-parameters.md)
1. [Project tests](/doc/sections/en_US/4-project-structure/4-4-project-tests.md)
    1. [Google Test](/doc/sections/en_US/4-project-structure/4-4-1-google-test.md)

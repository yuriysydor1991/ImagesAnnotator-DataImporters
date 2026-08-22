## The dataset importers API

The whole interface of the library is declared by the header files under [src/lib/facade/public](/src/lib/facade/public) and gets installed into the `include/ImagesAnnotatorDataImporters-0.12` sub-directory of the chosen install prefix. Nothing else leaves the shared object: every implementation class stays behind the abstract interfaces described here.

All the installable declarations live in the `ImagesAnnotatorDataImporters012` namespace. The name carries the library major and minor version numbers (`0.12` gives the `012` suffix) so that two library versions may coexist in a single translation unit. Alias it once in your own code:

```cpp
namespace iadi = ImagesAnnotatorDataImporters012;
```

The records the importers build are not defined here. They come from the [ImagesAnnotatorDataDrivers](https://github.com/yuriysydor1991/ImagesAnnotator-DataDrivers.git) library, whose interface namespace is `ImagesAnnotatorDataDrivers011` (aliased below as `iadd`). See the [The data drivers dependency](/doc/sections/en_US/5-project-build/5-36-the-data-drivers-dependency.md) subsection for the build side of that dependency.

### The installable header files

| Header | Declares |
| --- | --- |
| [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h) | the `IADI_API` visibility macro every installable declaration is marked with |
| [IImporter.h](/src/lib/facade/public/IImporter.h) | the `IImporter` abstract importer interface |
| [IImageSizeFacility.h](/src/lib/facade/public/IImageSizeFacility.h) | the `IImageSizeFacility` interface the consuming project implements |
| [LibraryContext.h](/src/lib/facade/public/LibraryContext.h) | the `LibraryContext` in and out data class both entry points are driven with |
| [PlainTxtImportLibraryContext.h](/src/lib/facade/public/contexts/PlainTxtImportLibraryContext.h) | the `LibraryContext` descendant of the plain text dataset layout |
| [Yolo4ImportLibraryContext.h](/src/lib/facade/public/contexts/Yolo4ImportLibraryContext.h) | the `LibraryContext` descendant of the YOLO v4 (darknet) dataset layout |
| [UltralyticsDetectImportLibraryContext.h](/src/lib/facade/public/contexts/UltralyticsDetectImportLibraryContext.h) | the `LibraryContext` descendant of the Ultralytics YOLO detection dataset layout |
| [UltralyticsObbImportLibraryContext.h](/src/lib/facade/public/contexts/UltralyticsObbImportLibraryContext.h) | the `LibraryContext` descendant of the Ultralytics YOLO oriented bounding box dataset layout |
| [UltralyticsSegmentImportLibraryContext.h](/src/lib/facade/public/contexts/UltralyticsSegmentImportLibraryContext.h) | the `LibraryContext` descendant of the Ultralytics YOLO instance segmentation dataset layout |
| [CocoImportLibraryContext.h](/src/lib/facade/public/contexts/CocoImportLibraryContext.h) | the `LibraryContext` descendant of the COCO object detection dataset layout |
| [PascalVocImportLibraryContext.h](/src/lib/facade/public/contexts/PascalVocImportLibraryContext.h) | the `LibraryContext` descendant of the Pascal VOC dataset layout |
| [CreateMLImportLibraryContext.h](/src/lib/facade/public/contexts/CreateMLImportLibraryContext.h) | the `LibraryContext` descendant of the Create ML object detection dataset layout |
| [PyTorchImportLibraryContext.h](/src/lib/facade/public/contexts/PyTorchImportLibraryContext.h) | the `LibraryContext` descendant of the PyTorch Vision dataset layout |
| [ILib.h](/src/lib/facade/public/ILib.h) | the `ILib` abstract library interface with its `perform_import` method |
| [LibraryFacade.h](/src/lib/facade/public/LibraryFacade.h) | the `LibraryFacade` factory class, the entry point of the library |

Including `LibraryFacade.h` pulls in every other header of the list.

### The dataset layouts

```cpp
class PlainTxtImportLibraryContext : public LibraryContext;
class Yolo4ImportLibraryContext : public LibraryContext;
class UltralyticsDetectImportLibraryContext : public LibraryContext;
class UltralyticsObbImportLibraryContext : public LibraryContext;
class UltralyticsSegmentImportLibraryContext : public LibraryContext;
class CocoImportLibraryContext : public LibraryContext;
class PascalVocImportLibraryContext : public LibraryContext;
class CreateMLImportLibraryContext : public LibraryContext;
class PyTorchImportLibraryContext : public LibraryContext;
```

The nine `LibraryContext` descendants name the nine dataset layouts the library is able to read. Instantiating one is what picks the layout, and the library maps that type onto the importer which reads it. None of them adds anything to `LibraryContext`, since everything an import needs - the source directory, the destination database and the image measuring instance - is held by the base class. What each of them expects to find on the disk is described in the [The read dataset layouts](/doc/sections/en_US/4-project-structure/4-10-the-read-dataset-layouts.md) subsection.

The nine are the reverse of the nine layouts the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library writes - one for one, under the very same names - and the pair round trips: a project exported into any of them and imported back yields the annotations it started with, up to what the layout itself is able to carry.

The five which can not be read without knowing how large each image is - see `IImageSizeFacility` below - are `Yolo4ImportLibraryContext`, the three `Ultralytics*ImportLibraryContext` ones, whose geometry is stored divided by that size, and `PyTorchImportLibraryContext`, whose annotation is the cropped image itself. `CocoImportLibraryContext` and `PascalVocImportLibraryContext` read the dimensions out of their own descriptors, and `PlainTxtImportLibraryContext` and `CreateMLImportLibraryContext` need no measurement at all, since both store their rectangles in the image own pixels, exactly as the internal project format keeps them.

### LibraryContext

The single data class of the library, the one both of its entry points are driven with: the one shot `ILib::perform_import()` and the `IImporter::import_db()` of an importer built by hand. Create it with the `LibraryFacade` factory method of the wanted layout - the nine `create_*_library_context()` ones listed below - or by instantiating that descendant yourself, which is what a consumer templated over the layout type does.

The data it carries is private and reached through accessors only. Every getter hands out a `const` reference to what the context holds, every setter copies the given value in:

| Accessors | Direction | Meaning |
| --- | --- | --- |
| `get_import_path()`, `set_import_path()` | in | the source directory of the import, a `std::string`, mandatory |
| `get_db()`, `set_db()` | in-out | the annotations database the recovered records are merged into, an `ImagesAnnotatorDataDrivers011::IAnnotationsDBPtr`, mandatory |
| `get_image_sizer()`, `set_image_sizer()` | in | the image measuring service, an `IImageSizeFacilityPtr`. Mandatory for the layouts which can not be read without it, unless the library was built with OpenCV |
| `get_importer()`, `set_importer()` | out | the importer instance the last `perform_import` ran, an `IImporterPtr` |
| `get_imported_records()`, `set_imported_records()` | out | how many image records the last import recovered and handed to the database, a `std::size_t` |

`LibraryContextPtr` is the `std::shared_ptr<LibraryContext>` alias.

The database is an `IAnnotationsDB` and not the read only `IImagesPathsDBProvider` the exporters library takes, because an import has to **add** records: they are handed over through `IAnnotationsDB::add_images_db()`, which skips an image the database already holds and keeps the result sorted by the image paths. Importing one and the same dataset twice therefore adds its images once, and importing into the database of a project being edited appends to it rather than replacing it. Obtain an empty one from `iadd::LibraryFacade::create_annotations_db()`.

`get_imported_records()` counts what the import recovered out of the dataset, not the growth of the database - the two differ by the records the merge above found already there.

### IImporter

```cpp
virtual bool import_db(LibraryContextPtr ictx) = 0;
```

The single method of an importer. It reads the dataset named by the context in the layout that importer implements, merges the records it recovers into the database of that very same context and returns `true` when the run as a whole went through. Entries it cannot process - a malformed line, an image file that is not there, a picture it cannot measure - are skipped and reported through the library log, they do not fail the run. A `false` means the context is incomplete, the import path holds no directory, or the directory holds no dataset of that layout at all - the descriptor the layout is built around missing or unreadable. The database is left untouched then.

The layout the context type names is not looked at here: the layout read is the one of the importer itself, so even the `LibraryContext` base class, which names no layout of its own, drives an `import_db()` just fine. An implementation must not keep the context beyond the call, since a context holding that importer back would close a pointer cycle. `IImporterPtr` is the `std::shared_ptr<IImporter>` alias.

### IImageSizeFacility

This is the one interface a consuming project may have to implement itself:

```cpp
virtual bool read_image_size(const std::string& imagePath, int& width,
                             int& height) = 0;
virtual IImageSizeFacilityPtr clone() = 0;
```

The library decodes no image format of its own, and an annotation of the internal project format is a rectangle in the pixels of the image it was drawn over. A dataset which stores its boxes divided by that size - the YOLO v4 one and the three Ultralytics YOLO ones here - therefore can not be read back without the size of the image each box belongs to, and the PyTorch Vision layout, whose whole annotation is the cropped image itself, can not be read back without it either. So the imports that need such a measurement ask their consumer to take it over whatever imaging stack that project already links.

The four remaining layouts are readable without one. The COCO and the Pascal VOC descriptors carry the size of every image they name, and the plain text and the Create ML ones hold their rectangles in the image own pixels, so a reader only fills the `iwidth` and `iheight` of the produced records in - which is worth handing over all the same, since those two fields are what the annotator draws a picture at its own scale with.

A library built with OpenCV ships an implementation of its own, so this interface only has to be implemented by a project that wants its own measuring - or by one consuming a library built without OpenCV. See [Enabling the OpenCV image size reader](/doc/sections/en_US/5-project-build/5-37-enabling-the-OpenCV-image-size-reader.md).

- `imagePath` is the filesystem path of the image to measure. The interface deliberately takes a path and not an image record: the record is what the import is building, and its size fields are what this call fills in.
- `width` and `height` are the out parameters to fill with the intrinsic pixel size of that file.
- Return `true` once both have been read. Returning `false` for an unreadable or an unsupported file is not an error of its own: the import logs that one image and carries on with the rest.

`clone()` has to produce a copy sharing no mutable decoding state with the original. The importers shipped today only ever call `read_image_size()`, but the method is a part of the interface and has to be implemented.

### ILib

`ILib::perform_import(LibraryContextPtr ctx)` is the single shot entry point of the library. It builds the importer of the layout the context names, publishes it through `ctx->set_importer()` and runs the import over that very same context, whose `get_imported_records()` then carries the count. It returns `false` when the context names no known layout or the import itself failed. Projects that want a finer grained control should rather build the importer directly with `LibraryFacade::create_importer()` and call `import_db()` on it - with the same context in both hands.

### LibraryFacade

A class of static factory methods only, and the only entry point a consuming project needs:

| Method | Returns |
| --- | --- |
| `create_plain_txt_library_context()` | a new empty `PlainTxtImportLibraryContextPtr` |
| `create_yolo4_library_context()` | a new empty `Yolo4ImportLibraryContextPtr` |
| `create_ultralytics_detect_library_context()` | a new empty `UltralyticsDetectImportLibraryContextPtr` |
| `create_ultralytics_obb_library_context()` | a new empty `UltralyticsObbImportLibraryContextPtr` |
| `create_ultralytics_segment_library_context()` | a new empty `UltralyticsSegmentImportLibraryContextPtr` |
| `create_coco_library_context()` | a new empty `CocoImportLibraryContextPtr` |
| `create_pascal_voc_library_context()` | a new empty `PascalVocImportLibraryContextPtr` |
| `create_createml_library_context()` | a new empty `CreateMLImportLibraryContextPtr` |
| `create_pytorch_library_context()` | a new empty `PyTorchImportLibraryContextPtr` |
| `create_default_lib()` | the default `ILibPtr` implementation |
| `create_library(LibraryContextPtr ctx)` | the `ILibPtr` implementation appropriate for the given context |
| `create_importer(const LibraryContextPtr& ctx)` | a new `IImporterPtr` for the layout of the context, or a `nullptr` for a context naming no known layout |
| `create_image_sizer()` | the image size reader the library ships itself, or a `nullptr` in a build without OpenCV |
| `library_version()` | the version string of the library binary in use |

### A complete example

The program below reads a YOLO v4 training directory back and writes the recovered annotations out as an ImagesAnnotator project file through the data drivers library. See the [Using the library in your project](/doc/sections/en_US/8-using-the-library-in-your-project/8-using-the-library-in-your-project.md) section for the CMake side of it.

```cpp
#include <ImagesAnnotatorDataDrivers-0.11/LibraryFacade.h>
#include <ImagesAnnotatorDataImporters-0.12/LibraryFacade.h>

#include <iostream>
#include <memory>

namespace iadd = ImagesAnnotatorDataDrivers011;
namespace iadi = ImagesAnnotatorDataImporters012;

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <dataset dir> <project.json>\n";
    return 1;
  }

  auto db = iadd::LibraryFacade::create_annotations_db();

  if (db == nullptr) {
    std::cerr << "fail to create the annotations database\n";
    return 1;
  }

  auto ctx = iadi::LibraryFacade::create_yolo4_library_context();

  ctx->set_import_path(argv[1]);
  ctx->set_db(db);

  auto importer = iadi::LibraryFacade::create_importer(ctx);

  if (importer == nullptr) {
    std::cerr << "no importer available for the requested layout\n";
    return 1;
  }

  if (!importer->import_db(ctx)) {
    std::cerr << "the import has failed\n";
    return 1;
  }

  if (!db->store_db(argv[2])) {
    std::cerr << "fail to store the project file " << argv[2] << '\n';
    return 1;
  }

  std::cout << "imported " << ctx->get_imported_records()
            << " image records by the importers library version "
            << iadi::LibraryFacade::library_version() << '\n';

  return 0;
}
```

No image size reader is handed over here, so the program above needs a library built with OpenCV. Give one of your own to `ctx->set_image_sizer()` otherwise - the next subsection shows how - or start from one of the four layouts which need none: the plain text, the COCO, the Pascal VOC and the Create ML ones.

Note that an existing project file may be imported **into**: open it with `iadd::LibraryFacade::open_annotations_db()` instead of creating an empty database, and the import appends the images the project does not hold yet.

### Implementing an image size reader

The sketch below wires one up over the imaging routines the consuming project already has - substitute your own decoding calls for the `my_imaging` ones:

```cpp
#include <ImagesAnnotatorDataImporters-0.12/IImageSizeFacility.h>

#include <memory>
#include <string>

namespace iadi = ImagesAnnotatorDataImporters012;

class MySizer : public iadi::IImageSizeFacility
{
 public:
  bool read_image_size(const std::string& imagePath, int& width,
                       int& height) override
  {
    auto image = my_imaging::load(imagePath);

    if (!image) {
      return false;
    }

    width = my_imaging::width_of(image);
    height = my_imaging::height_of(image);

    return width > 0 && height > 0;
  }

  IImageSizeFacilityPtr clone() override
  {
    return std::make_shared<MySizer>();
  }
};
```

Hand the instance over through the context and every importer picks it up:

```cpp
auto ctx = iadi::LibraryFacade::create_yolo4_library_context();

ctx->set_image_sizer(std::make_shared<MySizer>());
```

A reader set this way always wins over the one a library built with OpenCV ships, so a project that already decodes images its own way keeps doing exactly that.

### The IADI_API marker

Every installable class is marked with the `IADI_API` macro of [ImportersAPI.h](/src/lib/facade/public/ImportersAPI.h), and the library is compiled with the hidden symbol visibility, so that only the marked interface leaves the shared object. That is a correctness requirement rather than a size optimisation: the data drivers library this one links against is built from the very same project template and carries its own `default_logger::DefaultLogger` and `project_decls` definitions. Were both sets exported, the dynamic linker would be free to bind one library's calls into the other library's differently laid out classes.

The hiding does not reach the factory. A `std::make_shared` instantiation names its class in its own mangled name and stays weak and exported whatever the visibility is, so the implementation namespace here is `iadi0impl` and not the `lib0impl` the project template - and the data drivers library with it - uses.

A working consumer of the whole interface is kept in the tree as the `CTEST_Importers` component test, [src/importers/tests/component/Importers/CTEST_Importers.cpp](/src/importers/tests/component/Importers/CTEST_Importers.cpp): it links the real shared library and drives it through the public headers only, exactly the way a downstream project does.

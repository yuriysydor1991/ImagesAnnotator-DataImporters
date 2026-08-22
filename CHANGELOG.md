# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Run the test suite as a step of the quick build scripts, through the new [scripts/build/debug-test.sh](/scripts/build/debug-test.sh) adapted from the `--test` parameter of the [cpp-app-template](https://github.com/yuriysydor1991/cpp-app-template) this project was started from. It performs the `ctest --output-on-failure` call inside the `build/debug` directory and, like the install step beside it, does nothing at all unless that parameter was given, so a plain build stays a plain build. The five `Debug` entry point scripts which build the whole project - [debug.sh](/scripts/build/debug.sh), [debug-clang-tidy.sh](/scripts/build/debug-clang-tidy.sh), [debug-sanitizers.sh](/scripts/build/debug-sanitizers.sh), [debug-sanitizers-threads.sh](/scripts/build/debug-sanitizers-threads.sh) and [debug-compiler-analyzer.sh](/scripts/build/debug-compiler-analyzer.sh) - chain it in between of their build and install steps, which makes `scripts/build/debug.sh --test` the single command of a configure, a build and a whole test run; the [debug-cppcheck.sh](/scripts/build/debug-cppcheck.sh) one is left alone, since it builds one CMake target the test executables are not a part of. The `--test` parameter joins the `--no-reconfigure` and the `--install` ones in the `filter_script_args()` list of `scripts/build/common.sh`, so it is dropped from what the stage scripts hand over to `cmake` and to `ctest` while every other argument keeps reaching them - `scripts/build/debug-test.sh --test -R "CTEST_Importers\."` runs a part of the suite inside an already built directory.

- **Six more dataset layouts**, which brings the library up to the nine the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library writes - one importer for one exporter, under the very same layout names, so every directory that library produces is now readable back:
  - **The three Ultralytics YOLO layouts**, `UltralyticsDetectImportLibraryContext`, `UltralyticsObbImportLibraryContext` and `UltralyticsSegmentImportLibraryContext`, read by the `UltralyticsFolder2DBImporter` base and its three descendants of [src/importers/Ultralytics](/src/importers/Ultralytics). The `data.yaml` descriptor names the classes and the image directories - the `UltralyticsDataYaml` reader beside them takes all three spellings of its `names` entry - and every image of `images/` is paired with the label file its path names once the `images` element is swapped for `labels`. The detection line carries the box, the oriented bounding box one its four corners and the segmentation one the polygon outlining the object; the last two come back as the upright rectangle which holds what they draw, since the annotations database knows axis aligned rectangles only. All three are normalised by the size of their image, so all three need a measurement of it - and a coordinate outside the `0..1` range is cut down to the image, the way the exporting side cuts a rectangle reaching over an edge.
  - **The COCO object detection layout**, `CocoImportLibraryContext`, read by the `CocoFolder2DBImporter` of [src/importers/Coco](/src/importers/Coco). The `bbox` of an annotation is the `[x, y, width, height]` of its top left corner in the image own pixels - the very four fields an `ImageRecordRect` holds - and the descriptor carries the size of every image it names, so this layout needs no measurement at all. The descriptor is looked for under the `annotations/instances_default.json` the exporting side writes, then among the other `*.json` files of `annotations/` and of the import directory itself, so a dataset of a public release or of a tool handing out one flat directory is read where it lies.
  - **The Pascal VOC layout**, `PascalVocImportLibraryContext`, read by the `PascalVocFolder2DBImporter` of [src/importers/PascalVoc](/src/importers/PascalVoc). Every `*.xml` descriptor of `Annotations/` is read, and a directory carrying no such sub-directory is read as the flat one [LabelImg](https://github.com/HumanSignal/labelImg) saves its work in. The `bndbox` corners become the origin and the size, the `size` element fills the record dimensions in, and `ImageSets/Main` is not read at all: a project holds the images and no split of them.
  - **The Create ML object detection layout**, `CreateMLImportLibraryContext`, read by the `CreateMLFolder2DBImporter` of [src/importers/CreateML](/src/importers/CreateML). The `x` and `y` of a box are its centre in the image own pixels, which this undoes onto the corner the rectangle was drawn from - the half pixel of an odd sized box included. Both spellings of the two keys are read: the singular `imagefilename` and `annotation` pair Apple documents, and the plural one some converters emit.
- **The JSON and the XML document readers** those descriptors are read through, [src/parsers/json](/src/parsers/json) and [src/parsers/xml](/src/parsers/xml). The library links no JSON or XML library of its own, exactly as the sibling exporters library writes those very descriptors without one, and neither reader knows anything of the layouts read through it - each is driven through its own `IJsonParser` or `IXmlParser` interface and tested on its own documents, the malformed ones among them.
- `Folder2DBImporter::measure_unsized()` and `Folder2DBImporter::read_file()`, the two halves of an import the new layouts share with the older ones: the filling in of the image dimensions a layout carries no size for, which the plain text importer now takes from there as well, and the reading of a descriptor which is one document rather than a list of lines.
- `JsonDescriptor2DBImporter`, what the COCO and the Create ML importers share on top of that base: the locating of the one JSON document a dataset of either layout is described by, among the files it may arrive under.
- `StringHelper` and `Utf8Helper` beside the `TypeHelper` of [src/helpers](/src/helpers) - the trimming of the whitespace a descriptor entry is laid out with, which the YOLO v4 importer now takes from there as well, and the writing out of the code point a `\uXXXX` escape or a `&#NN;` reference names. `TypeHelper` gained the `toPixels()` rounding every layout writing a fraction is read back through.
- Six `LibraryFacade` factory methods for the new contexts - `create_ultralytics_detect_library_context()`, `create_ultralytics_obb_library_context()`, `create_ultralytics_segment_library_context()`, `create_coco_library_context()`, `create_pascal_voc_library_context()` and `create_createml_library_context()` - named after the ones the exporters library hands out for the very same layouts.

### Changed

- Gather the layout specific `LibraryContext` descendants into a `contexts` subdirectory of the installable public headers, the way the sibling [ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git) library gathers its own nine, so the two libraries keep the one layout and so `src/lib/facade/public` lists the six headers that make up the API of the library - `ImportersAPI.h`, `IImporter.h`, `IImageSizeFacility.h`, `ILib.h`, `LibraryContext.h` and `LibraryFacade.h` - on their own. The three move together with no rename: `PlainTxtImportLibraryContext.h`, `PyTorchImportLibraryContext.h` and `Yolo4ImportLibraryContext.h` are installed at `<prefix>/include/ImagesAnnotatorDataImporters-0.11/contexts/` from now on, and each of them reaches the headers it left behind through `../`, which is what keeps them resolving once installed - the include root of a consumer is `<prefix>/include`, so the sibling spelling would have looked for `<prefix>/include/LibraryContext.h`. **No class, method or namespace changes with it**, and neither does the way a consumer is meant to reach them: `LibraryFacade.h` includes all three, so a project including that one header alone - which is what the documented usage does, and what the ImagesAnnotator application does - needs no change whatsoever. Only a project that included a layout context header directly has to spell the `contexts/` component now. The base `LibraryContext.h` deliberately stays at the root of the public directory, since it is the type the `IImporter`, `ILib` and `LibraryFacade` signatures are written in and belongs to no single layout - and it is the one this library hangs the shared import path, database and image sizer on.

## [0.11.0] - 2026-08-17

The first release of `ImagesAnnotatorDataImporters` as a project of its own. The
repository started life as the `lib` branch of the
[cpp-app-template](https://github.com/yuriysydor1991/cpp-app-template) project
and has now been filled with the dataset importers - the exact counterpart of
the sibling
[ImagesAnnotator-DataExporters](https://github.com/yuriysydor1991/ImagesAnnotator-DataExporters.git)
library, so that a training dataset may be read back into an
[ImagesAnnotator](https://github.com/yuriysydor1991/ImagesAnnotator) annotations
project instead of only being written out of one.

The version number is the `0.11` of that pair rather than a `0.1` of a project
starting over: this library is one half of a set whose namespace, binary,
header directory and CMake package all carry the same major and minor version,
and both halves are meant to be installed together.

### Added

- **The plain text dataset layout**, `PlainTxtImportLibraryContext`, read by the
  `PlainTxtFolder2DBImporter` of [src/importers/PlainTxt](/src/importers/PlainTxt).
  Every `*.txt` file of the import directory is one annotation name, and every
  line of it names an image, the number of the rectangles of that name drawn
  over it and their `x y width height` in the image own pixels. An image named
  by more than one file collects the rectangles of all of them into one record.
- The image path of such a line is written unquoted and the layout names no
  separator, so **the rectangle count is what tells the path from the numbers
  behind it**: the reader takes the first token which is a whole number with
  exactly four numbers per rectangle following it. A path holding a space
  therefore round trips, and a line promising more or fewer rectangles than it
  wrote is dropped whole and logged.
- This is the one layout of the three which needs no measurement of the
  pictures, since it already stores its rectangles in the pixels the internal
  format keeps them in. An `IImageSizeFacility` handed over anyway only fills in
  the image dimensions of the produced records.
- **The YOLO v4 dataset layout**, `Yolo4ImportLibraryContext`, read by the
  `Yolo4Folder2DBImporter` of [src/importers/Yolo4](/src/importers/Yolo4). The
  darknet training directory is read whole: the class names of
  `data/obj.names`, whose line number is the class index every label file
  writes, the image list of `data/train.txt` and one `<image stem>.txt` label
  file per image, each line of which carries a class index and the box centre
  and size divided by the size of that image.
- The `data/obj.data` descriptor is read when it is there - at that path and at
  the `obj.data` beside the import directory, since the darknet examples keep it
  in either place - and its `names` and `train` entries then decide which files
  the two lists are taken from, so a dataset laid out by some other tool is
  followed rather than assumed. Without the descriptor the conventional relative
  paths are used, without a training list the `valid` one is tried, and without
  either the `data` directory itself is walked.
- The division is undone with the measured size of every image: the centre and
  the extent are multiplied back, the origin is the centre less half of that
  extent, and each of the four is rounded to the nearest pixel. A rectangle
  written by the exporters library comes back as the very rectangle it was
  written from. An image which stays unmeasured is skipped whole, since there is
  nothing left to place its boxes by, and an image with no label file beside it
  becomes a record with no rectangles - darknet reads such an image as a picture
  holding none of the classes.
- The `cfg/yolov4-obj.cfg` network descriptor holds nothing of the annotations
  and is not read at all.
- **The PyTorch Vision dataset layout**, `PyTorchImportLibraryContext`, read by
  the `PyTorchVisionFolder2DBImporter` of
  [src/importers/PyTorch](/src/importers/PyTorch). Every immediate
  sub-directory of the import directory is a class, and every file inside it is
  a cropped image. That layout has lost the pictures the crops were cut out of,
  so each crop becomes an image record of its own carrying the one rectangle
  over its whole area, named after the directory it was found in. This is the
  one layout of the three which does not round trip into the records it was
  written from, and the class of every crop survives in full, which is what the
  layout is for.
- **`IImageSizeFacility`**, the one interface a consuming project may have to
  implement itself. The library decodes no image format of its own, and an
  annotation of the internal project format is a rectangle in the pixels of its
  image, so the layouts which do not store those pixels ask their consumer to
  measure the picture over whatever imaging stack it already links. It is the
  counterpart of the `IImageCropperFacility` of the exporters library, and it
  deliberately takes a path rather than an image record: the record is what the
  import is building.
- The facility sits on `LibraryContext` itself rather than on a single layout
  descendant, unlike the cropper of the exporters library: two of the three
  layouts read here need a measurement, and the third fills the image
  dimensions of its records with it when it is there.
- **The library's own image size reader**, `OpenCVImageSizer` of
  [src/sizers](/src/sizers), compiled into a build which found OpenCV and handed
  out by `LibraryFacade::create_image_sizer()`. A reader the consumer sets
  always wins over it, so a project that already decodes images its own way
  keeps doing exactly that, and a consumer with no imaging stack still gets
  every layout. The `ENABLE_OPENCV` option means *probe*, not *require*: a
  system without OpenCV configures and builds just the same.
- **`LibraryContext`** carries the import path, the destination database, the
  image measuring instance, and reports the importer that ran and the number of
  the records it recovered. The destination is an
  `ImagesAnnotatorDataDrivers011::IAnnotationsDBPtr` and not the read only
  `IImagesPathsDBProvider` the exporters library takes, because an import has to
  add records: they are handed over through `IAnnotationsDB::add_images_db()`,
  which keeps an image the database already holds. So one and the same dataset
  imported twice adds its images once, and a dataset imported into the database
  of a project being edited appends to it.
- **`Folder2DBImporter`**, the shared half of every importer: the guards over
  the context, the resolving of the image measuring instance, the merge into the
  database and the count reported back all live there, and what tells the
  importers apart is the one `read_dataset()` method it asks them for. An
  importer of a layout that cannot be read without a measurement says so through
  `needs_image_sizer()`, and such an import fails at once instead of recovering
  nothing.
- **The installable interface** of
  [src/lib/facade/public](/src/lib/facade/public): `LibraryFacade` with a
  context factory method per layout, `ILib::perform_import()` as the one shot
  entry point, `IImporter::import_db()` for a finer grained control, the three
  layout contexts, `IImageSizeFacility` and the `IADI_API` marker of
  `ImportersAPI.h`. Everything else stays inside the shared object, which is
  built with the hidden symbol visibility, and the implementation namespace is
  `iadi0impl` rather than the template's `lib0impl` so that the weak
  `std::make_shared` instantiations of the two libraries never collide.
- **The tests**: `UTEST_PlainTxtFolder2DBImporter`,
  `UTEST_Yolo4Folder2DBImporter`, `UTEST_PyTorchVisionFolder2DBImporter`,
  `UTEST_OpenCVImageSizer`, `UTEST_LibraryFacade`, `UTEST_LibFactory`,
  `UTEST_LibMain` and `UTEST_TypeHelper` as the unit ones, `CTEST_Importers`,
  `CTEST_LibraryRealLogger` and `CTEST_DefaultLogger` as the component ones -
  90 cases in total. The importer suites build their dataset directories under
  the Google Test temporary directory and drive stand-in image size readers, one
  reporting a fixed size and one failing every call, so every path is covered
  whether or not the build found OpenCV. `CTEST_Importers` compiles nothing of
  the library and drives the produced shared object through the installable
  headers only.
- **The documentation** of the whole interface and of the three layouts, in
  English and Ukrainian, with the class diagram of the `lib` component.

### Removed

Compared with the `lib` branch of the project template this repository was
started from, and with the sibling exporters library it mirrors:

- the libcurl integration and the `ImageLoader` around it. The exporters preload
  a web hosted image record before they copy or crop it; an import reads a
  dataset directory that is already on the disk, so neither is needed here.
- the `ImageRecordUrlAndPathHelper`, which existed for that preloading only.
- the compression, image, libxml2, nlohmann-json and OpenSSL enablers of the
  template, the flatpak packager manifest and the integration test tree - none
  of which this library has anything to put into.

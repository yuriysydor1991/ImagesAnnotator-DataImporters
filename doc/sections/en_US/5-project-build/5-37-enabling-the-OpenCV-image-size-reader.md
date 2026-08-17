## Enabling the OpenCV image size reader

The library decodes no image format of its own. That is why `LibraryContext::set_image_sizer()` exists: an annotation of the internal project format is a rectangle in the pixels of the image it was drawn over, so a dataset which does not store those pixels can not be read back without measuring the picture. The library asks its consumer to take that measurement over whatever imaging stack that project already links - the GTKmm, the Qt or the wxWidgets loader of the ImagesAnnotator application, for instance.

A consumer that has no imaging stack to lend was stuck. So, when the build finds OpenCV, the library also ships an image size reader of its own, and that consumer needs to supply nothing.

Two of the three [dataset layouts](/doc/sections/en_US/4-project-structure/4-10-the-read-dataset-layouts.md) are unreadable without such a reader: the YOLO v4 one, whose boxes are divided by the image size, and the PyTorch Vision one, whose annotation is the size of the cropped file itself. The plain text layout needs none and only fills the image dimensions in when a reader happens to be there.

### The option

```
cmake -S . -B build -DENABLE_OPENCV=ON
```

`ENABLE_OPENCV` defaults to `ON`, and `ON` means *probe*, not *require*. The dependency is optional in both directions:

| Situation | Result |
| --- | --- |
| OpenCV is installed | the reader is compiled in and the library links `core` and `imgcodecs` |
| OpenCV is not installed | the configure succeeds, prints why, and the library is built without the reader |
| `-DENABLE_OPENCV=OFF` | the probe is skipped entirely, with the same result |

Nothing fails a configure over it. A missing OpenCV only means the imports keep asking their consumer for a reader.

Point the probe at a prefix of your own with `-DOpenCV_DIR=<dir>` when the installation is not in a system default place, and narrow or widen the components it asks for with `-DTEMPLATE_APP_OPENCV_COMPONENTS="core;imgcodecs"`.

### Which reader an import uses

The rule is one line: **a reader you supply always wins.**

1. A reader was handed over through `LibraryContext::set_image_sizer()` - that reader is used. A project that already decodes images its own way keeps doing exactly that, whether the library has OpenCV or not.
1. The slot was left empty and the library has the OpenCV reader - the import uses it, and you had to configure nothing.
1. The slot was left empty and the library has no reader - an import of a layout that needs one fails at once with a log line saying so. An import of the plain text layout runs on and only leaves the image dimensions of its records at zero.

So a new consumer may simply leave the field alone.

### Asking for it directly

`LibraryFacade::create_image_sizer()` hands out the same reader, or a `nullptr` in a build without OpenCV. It is the way to find out whether a build has one:

```cpp
namespace iadi = ImagesAnnotatorDataImporters011;

auto ctx = iadi::LibraryFacade::create_yolo4_library_context();
ctx->set_import_path("/tmp/yolo-dataset");
ctx->set_db(db);

// Optional. Leaving the field alone gets the very same reader.
ctx->set_image_sizer(iadi::LibraryFacade::create_image_sizer());

if (ctx->get_image_sizer() == nullptr) {
  // This build has no OpenCV - supply a reader of your own, or pick an
  // import format that needs none.
}
```

### What it does

`read_image_size()` decodes the file at the given path and reports its width and its height:

- a file it cannot decode, and one that is not there at all, both fail the call, which costs that one image and not the import run;
- the two out parameters are left untouched on a failure, so a caller never sees half a measurement;
- the whole picture is decoded to learn two numbers, since the OpenCV C++ API exposes no header only probe. The reduced `IMREAD_` flags would decode less but report the reduced size, and a box scaled by such a guess is a box in the wrong place.

No OpenCV type appears in any installed header. The reader is reached through the abstract `IImageSizeFacility` like any consumer supplied one, so **a project consuming this library needs no OpenCV of its own**, even when the library was built with it.

### Tests

`UTEST_OpenCVImageSizer` covers the reader and is configured only in a build that found OpenCV, since every one of its cases drives it. The build without OpenCV is covered by `UTEST_LibFactory.create_image_sizer_matches_what_the_build_found`, which asserts that the factory hands out nothing there. The importer suites drive both a reader reporting a fixed size and one failing every call, so what an import makes of either is covered in every configuration.

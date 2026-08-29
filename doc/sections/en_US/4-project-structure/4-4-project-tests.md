## Project tests

The project carries two kinds of tests, each behind its own CMake option.

The unit tests are built with `-DENABLE_UNIT_TESTS=ON`. They compile the components straight from the tree against the gmock stand-ins under [src/tests/mocks](/src/tests/mocks), `src/lib/facade/tests/mocks` and `src/lib/libmain/tests/mocks`, so a component is exercised with its neighbours replaced:

| Binary | Under test | Sources |
|---|---|---|
| `UTEST_IADataImportersFacade` | the installed facade | [src/lib/facade/IADataImportersFacade.cpp](/src/lib/facade/IADataImportersFacade.cpp) |
| `UTEST_LibFactory` | the object factory | [src/lib/libmain/LibFactory.cpp](/src/lib/libmain/LibFactory.cpp) |
| `UTEST_LibMain` | the `IADataImportersLib` implementation | [src/lib/libmain/LibMain.cpp](/src/lib/libmain/LibMain.cpp) |
| `UTEST_PlainTxtFolder2DBImporter` | the plain text importer | [src/importers/PlainTxt/PlainTxtFolder2DBImporter.cpp](/src/importers/PlainTxt/PlainTxtFolder2DBImporter.cpp) |
| `UTEST_Yolo4Folder2DBImporter` | the darknet YOLOv4 importer | [src/importers/Yolo4/Yolo4Folder2DBImporter.cpp](/src/importers/Yolo4/Yolo4Folder2DBImporter.cpp) |
| `UTEST_UltralyticsFolder2DBImporter` | the three Ultralytics YOLO importers and the `data.yaml` reader they share | [src/importers/Ultralytics](/src/importers/Ultralytics) |
| `UTEST_CocoFolder2DBImporter` | the COCO importer | [src/importers/Coco/CocoFolder2DBImporter.cpp](/src/importers/Coco/CocoFolder2DBImporter.cpp) |
| `UTEST_PascalVocFolder2DBImporter` | the Pascal VOC importer | [src/importers/PascalVoc/PascalVocFolder2DBImporter.cpp](/src/importers/PascalVoc/PascalVocFolder2DBImporter.cpp) |
| `UTEST_CreateMLFolder2DBImporter` | the Create ML importer | [src/importers/CreateML/CreateMLFolder2DBImporter.cpp](/src/importers/CreateML/CreateMLFolder2DBImporter.cpp) |
| `UTEST_PyTorchVisionFolder2DBImporter` | the PyTorch vision importer | [src/importers/PyTorch/PyTorchVisionFolder2DBImporter.cpp](/src/importers/PyTorch/PyTorchVisionFolder2DBImporter.cpp) |
| `UTEST_JsonParser` | the JSON document reader and the value tree it produces | [src/parsers/json](/src/parsers/json) |
| `UTEST_XmlParser` | the XML document reader and the element tree it produces | [src/parsers/xml](/src/parsers/xml) |
| `UTEST_OpenCVImageSizer` | the library's own image size reader, configured only in a build that found OpenCV | [src/sizers/OpenCVImageSizer.cpp](/src/sizers/OpenCVImageSizer.cpp) |
| `UTEST_TypeHelper` | the numeric cast and rounding helper | [src/helpers/TypeHelper.h](/src/helpers/TypeHelper.h) |
| `UTEST_StringHelper` | the descriptor value trimming helper | [src/helpers/StringHelper.h](/src/helpers/StringHelper.h) |
| `UTEST_Utf8Helper` | the code point writing helper | [src/helpers/Utf8Helper.h](/src/helpers/Utf8Helper.h) |

The seven importer suites also cover the shared [src/importers/Folder2DBImporter.cpp](/src/importers/Folder2DBImporter.cpp) base, since every one of them compiles and drives it, and the two JSON described layouts cover the [src/importers/JsonDescriptor2DBImporter.cpp](/src/importers/JsonDescriptor2DBImporter.cpp) half they share on top of it. Each of them runs a stand-in image size reader of its own - one reporting a fixed size and one failing every call - so what an import makes of either is covered whether or not this build has OpenCV. The two parser suites compile no importer at all: neither reader knows anything of the layouts read through it, so both are driven with documents of their own, the malformed ones among them. Every case works over a directory tree the fixture builds under the Google Test temporary directory and removes again, so the suite touches nothing else and needs no test data of its own.

The component tests are built with `-DENABLE_COMPONENT_TESTS=ON`:

| Binary | Scope |
|---|---|
| `CTEST_DefaultLogger` | the logger compiled into the library, driven through its own class |
| `CTEST_LibraryRealLogger` | the adoption of the consumer logger, driven through the produced shared object |
| `CTEST_Importers` | compiles none of the library: it links the produced shared object and drives it through the installable public headers only, exactly the way a downstream project does |

`CTEST_Importers` is the test that would notice a broken install interface - a header left out of the install set, or a symbol that never left the shared object because it lost its `IADI_API` mark.

With both options on the suite counts 186 test cases:

```
# from the project root directory

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=<the data drivers install prefix> \
  -DENABLE_UNIT_TESTS=ON -DENABLE_COMPONENT_TESTS=ON
cmake --build build -j$(nproc)
ctest --test-dir build
```

Every test is registered by `gtest_add_tests()` with a `.noArgs` suffix and a 600 second timeout. See [Enabling unit testing](/doc/sections/en_US/5-project-build/testing/5-3-1-enabling-unit-testing.md) for the build options and [Run tests by the ctest](/doc/sections/en_US/6-running-the-tests/6-3-1-run-tests-by-the-ctest.md) for running and filtering them.

### Manual tests run

Every test binary is an ordinary Google Test executable, so it may be started
on its own without `ctest`. The tests are separated into different files named
with the `UTEST_*` pattern for the unit tests and with the `CTEST_*` pattern
for the component ones.

You may search for all compiled available tests by a next command:

```
# from the project root or a build directory

find -type f -executable \( -name 'UTEST_*' -o -name 'CTEST_*' \)
```

After that, choose the particular test of interest and execute it manually if
needed. For example, for the `LibraryFacade` class pass its unit test file
system path into the command line and hit enter (GNU/Linux based):

```
# from the project root

./build/src/lib/facade/tests/unit/LibraryFacade/UTEST_LibraryFacade
```

The importer tests live next to the sources they cover:

```
# from the project root

./build/src/importers/PlainTxt/tests/unit/PlainTxtFolder2DBImporter/UTEST_PlainTxtFolder2DBImporter
./build/src/importers/Yolo4/tests/unit/Yolo4Folder2DBImporter/UTEST_Yolo4Folder2DBImporter
./build/src/importers/PyTorch/tests/unit/PyTorchVisionFolder2DBImporter/UTEST_PyTorchVisionFolder2DBImporter
```

The `CTEST_Importers` component test is started the same way. Unlike the unit
tests it links the produced shared library and reaches it through the public
headers only, exactly the way a downstream project does, so it exercises the
real importers end to end:

```
# from the project root

./build/src/importers/tests/component/Importers/CTEST_Importers
```

The usual Google Test command line flags apply, for instance to run a single
case or to list the available ones:

```
./build/src/lib/facade/tests/unit/LibraryFacade/UTEST_LibraryFacade --gtest_list_tests
./build/src/lib/facade/tests/unit/LibraryFacade/UTEST_LibraryFacade --gtest_filter=*library_version*
```

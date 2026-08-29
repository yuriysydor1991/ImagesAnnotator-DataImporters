## Documentation build

Besides the hand written sections under `doc/sections`, the project can
auto-generate an API reference with the Doxygen tool from the comments of the
sources and of the installable public headers.

To add the Doxygen CMake target during the configure stage set the
`ENABLE_DOC_DOXYGEN` CMake variable to `ON` (GNU/Linux based):

```
# inside the project root directory

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DENABLE_DOC_DOXYGEN=ON
```

The configure fails when no Doxygen is installed, because
[doc/CMakeLists.txt](/doc/CMakeLists.txt) resolves it with
`find_package(Doxygen REQUIRED)` once the option is on.

Then build the documentation with:

```
# inside the project root directory

cmake --build build --target Doxygen-doc
```

That generates the `doc/ImagesAnnotatorDataImporters-0.14-html` directory (the
`doc/*-html` pattern is already in `.gitignore`) holding the HTML
documentation; open its `index.html` file to examine the result.

The directory name is `<PROJECT_LIBRARY_NAME>-html`, so it follows the
installable library name: a configure with
`-DLIB_INCLUDE_MINOR_IN_NAME=ON` produces
`doc/ImagesAnnotatorDataImporters-0.14-html` instead - see
[Customizing the installable library name segments](/doc/sections/en_US/5-project-build/5-23-customizing-library-name-segments.md).
The name can also be set directly with the `DOXYGEN_OUT_HTML_NAME` cache
variable, and the parent directory with `DOXYGEN_OUTPUT_DIR` (it defaults to
the `doc` directory of the source tree).

The output language is selected with `DOXYGEN_OUTPUT_LANGUAGE`, which defaults
to `English`. Setting it to `Ukrainian` also switches the documentation home
page to `doc/README.uk_UA.md`; otherwise the page is taken from the file named
by `DOXYGEN_MAIN_PAGE_MD`, by default the top level `README.md`.

```
# inside the project root directory

cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/data-drivers-install-prefix \
  -DENABLE_DOC_DOXYGEN=ON \
  -DDOXYGEN_OUTPUT_LANGUAGE=Ukrainian
cmake --build build --target Doxygen-doc
```

The [doc/Doxyfile.in](/doc/Doxyfile.in) template holds every remaining Doxygen
parameter and may be edited to change the generated output.

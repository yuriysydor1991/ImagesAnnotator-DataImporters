cmake_minimum_required(VERSION 3.13)

include(template-project-compile-options)
add_compile_options(${EXTRA_COMPILE_OPTIONS})
add_link_options(${EXTRA_LINK_OPTIONS})

configure_file(
  project-global-decls.h.in
  ${CMAKE_BINARY_DIR}/project-global-decls.h
)

set(GENERAL_MOCKS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/tests/mocks")

# Per-component source lists shared with the test executables. The unit tests
# compile the components straight from the tree (against the mocks under
# src/tests/mocks) instead of linking the produced shared library, so they need
# to name the very same translation units. Declared here, before any
# subdirectory is added, so that every test tree below can see them.
# One sub-directory per dataset layout, so a unit test names the sources of
# its own layout only, while the whole list stays available for the tests
# reaching every importer at once.
set(IADI_IMPORTERS_SRC_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/importers
)

# What every layout shares: the guards, the image measuring instance and the
# merging of the recovered records into the database. Every importer source
# list below carries it, since every importer descends from it.
set(IADI_IMPORTERS_COMMON_SRC
  ${IADI_IMPORTERS_SRC_DIR}/Folder2DBImporter.cpp
)

set(IADI_PLAIN_TXT_SRC
  ${IADI_IMPORTERS_COMMON_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/PlainTxt/PlainTxtFolder2DBImporter.cpp
)

set(IADI_YOLO4_SRC
  ${IADI_IMPORTERS_COMMON_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/Yolo4/Yolo4Folder2DBImporter.cpp
)

set(IADI_PYTORCH_SRC
  ${IADI_IMPORTERS_COMMON_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/PyTorch/PyTorchVisionFolder2DBImporter.cpp
)

# The two document readers the layouts whose whole descriptor is one file are
# read through. They are held apart from the importers, since more than one
# layout shares each of them, and they are tested apart from those as well.
set(IADI_PARSERS_SRC_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/parsers
)

set(IADI_JSON_PARSER_SRC
  ${IADI_PARSERS_SRC_DIR}/json/JsonValue.cpp
  ${IADI_PARSERS_SRC_DIR}/json/JsonParser.cpp
)

set(IADI_XML_PARSER_SRC
  ${IADI_PARSERS_SRC_DIR}/xml/XmlNode.cpp
  ${IADI_PARSERS_SRC_DIR}/xml/XmlParser.cpp
)

# What the two layouts described by a single JSON document share on top of it:
# the locating of that document among the files a dataset of the layout may
# arrive with.
set(IADI_JSON_DESCRIPTOR_SRC
  ${IADI_IMPORTERS_COMMON_SRC}
  ${IADI_JSON_PARSER_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/JsonDescriptor2DBImporter.cpp
)

set(IADI_COCO_SRC
  ${IADI_JSON_DESCRIPTOR_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/Coco/CocoFolder2DBImporter.cpp
)

set(IADI_CREATEML_SRC
  ${IADI_JSON_DESCRIPTOR_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/CreateML/CreateMLFolder2DBImporter.cpp
)

set(IADI_PASCAL_VOC_SRC
  ${IADI_IMPORTERS_COMMON_SRC}
  ${IADI_XML_PARSER_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/PascalVoc/PascalVocFolder2DBImporter.cpp
)

# The three Ultralytics YOLO layouts differ in one label file line only, so the
# three importers of them are one source list: a unit test driving any of them
# compiles all three.
set(IADI_ULTRALYTICS_SRC
  ${IADI_IMPORTERS_COMMON_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsDataYaml.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsDetectFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsObbFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsSegmentFolder2DBImporter.cpp
)

set(IADI_IMPORTERS_SRC
  ${IADI_IMPORTERS_COMMON_SRC}
  ${IADI_JSON_PARSER_SRC}
  ${IADI_XML_PARSER_SRC}
  ${IADI_IMPORTERS_SRC_DIR}/JsonDescriptor2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/PlainTxt/PlainTxtFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Yolo4/Yolo4Folder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/PyTorch/PyTorchVisionFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Coco/CocoFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/CreateML/CreateMLFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/PascalVoc/PascalVocFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsDataYaml.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsDetectFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsObbFolder2DBImporter.cpp
  ${IADI_IMPORTERS_SRC_DIR}/Ultralytics/UltralyticsSegmentFolder2DBImporter.cpp
)

# The installed context carries the accessors of the import data, so every test
# executable driving a context needs this translation unit, exactly as the
# library target itself does.
set(IADI_FACADE_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/facade/LibraryContext.cpp
)

# The image size reader factory is compiled into every build; the OpenCV
# implementation behind it only into the ones that found OpenCV. Any test
# executable that compiles an importer needs this list, because every importer
# asks the factory for a reader when the context carries none.
set(IADI_SIZERS_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/sizers/ImageSizeFactory.cpp
)

# What such a test executable additionally needs to compile and link. All three
# are empty in a build without OpenCV, so the test declarations stay free of
# the build time condition.
set(IADI_OPENCV_TEST_INCLUDES "")
set(IADI_OPENCV_TEST_LIBS "")
set(IADI_OPENCV_TEST_DEFS "")

if (IADI_WITH_OPENCV)
  list(APPEND IADI_SIZERS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/sizers/OpenCVImageSizer.cpp)

  set(IADI_OPENCV_TEST_INCLUDES ${OpenCV_INCLUDE_DIRS})
  set(IADI_OPENCV_TEST_LIBS ${OpenCV_LIBS})
  set(IADI_OPENCV_TEST_DEFS IADI_WITH_OPENCV=1)
endif()

# The lib subdirectory declares the library target, so it comes first: every
# implementation component below registers its sources against that target.
add_subdirectory(lib)

add_subdirectory(helpers)
add_subdirectory(sizers)
add_subdirectory(parsers)
add_subdirectory(importers)

add_subdirectory(log)

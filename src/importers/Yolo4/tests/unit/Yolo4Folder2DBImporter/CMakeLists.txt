cmake_minimum_required(VERSION 3.13)

add_executable(
  UTEST_Yolo4Folder2DBImporter
  UTEST_Yolo4Folder2DBImporter.cpp
  ${IADI_YOLO4_SRC}
  ${IADI_FACADE_SRC}
  ${IADI_SIZERS_SRC}
)

target_include_directories(
  UTEST_Yolo4Folder2DBImporter
  PRIVATE ${GENERAL_MOCKS_ROOT}/log
  PRIVATE ${GENERAL_MOCKS_ROOT}/project-global-decls
  PRIVATE ${CMAKE_SOURCE_DIR}
  PRIVATE ${IADI_PUBLIC_INCLUDE_DIR}
  PRIVATE ${IADI_OPENCV_TEST_INCLUDES}
)

target_link_libraries(
  UTEST_Yolo4Folder2DBImporter
  GTest::gtest_main GTest::gmock
  ${IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET} ${IADI_OPENCV_TEST_LIBS}
)

target_compile_definitions(
  UTEST_Yolo4Folder2DBImporter
  PRIVATE ${IADI_OPENCV_TEST_DEFS}
)

include(GoogleTest)

gtest_add_tests(
  TARGET UTEST_Yolo4Folder2DBImporter
  TEST_SUFFIX .noArgs
  TEST_LIST noArgsTests
)

set_tests_properties(${noArgsTests} PROPERTIES TIMEOUT 600)

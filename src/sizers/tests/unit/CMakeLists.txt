cmake_minimum_required(VERSION 3.13)

if(NOT ENABLE_UNIT_TESTS)
  return()
endif()

# Declared only in a build that has OpenCV, because every case of the suite
# drives the OpenCV image size reader itself. gtest_add_tests() registers the
# cases by scanning the source text and does not honour the preprocessor, so a
# suite guarded by an #ifdef inside the file would register cases that the
# binary does not contain - and ctest reports a filter that matches nothing as
# a pass. The build without OpenCV is covered by UTEST_LibFactory instead,
# which asserts that the factory hands out no reader there.
if(NOT IADI_WITH_OPENCV)
  return()
endif()

# disabling the asserts for the tests
add_compile_options(-DNDEBUG=1)

add_subdirectory(OpenCVImageSizer)

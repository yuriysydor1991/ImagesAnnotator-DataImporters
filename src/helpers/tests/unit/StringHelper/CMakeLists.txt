cmake_minimum_required(VERSION 3.13)

add_executable(
  UTEST_StringHelper
  UTEST_StringHelper.cpp
)

target_include_directories(
  UTEST_StringHelper
  PRIVATE ${GENERAL_MOCKS_ROOT}/log
  PRIVATE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
  UTEST_StringHelper
  GTest::gtest_main GTest::gmock
  ${IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET}
)

include(GoogleTest)

gtest_add_tests(
  TARGET UTEST_StringHelper
  TEST_SUFFIX .noArgs
  TEST_LIST noArgsTests
)

set_tests_properties(${noArgsTests} PROPERTIES TIMEOUT 600)

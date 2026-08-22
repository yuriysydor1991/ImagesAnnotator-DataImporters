cmake_minimum_required(VERSION 3.13)

add_executable(
  UTEST_Utf8Helper
  UTEST_Utf8Helper.cpp
)

target_include_directories(
  UTEST_Utf8Helper
  PRIVATE ${GENERAL_MOCKS_ROOT}/log
  PRIVATE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
  UTEST_Utf8Helper
  GTest::gtest_main GTest::gmock
  ${IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET}
)

include(GoogleTest)

gtest_add_tests(
  TARGET UTEST_Utf8Helper
  TEST_SUFFIX .noArgs
  TEST_LIST noArgsTests
)

set_tests_properties(${noArgsTests} PROPERTIES TIMEOUT 600)

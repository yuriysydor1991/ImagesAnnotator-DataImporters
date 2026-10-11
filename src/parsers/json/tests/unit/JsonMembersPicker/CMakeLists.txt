cmake_minimum_required(VERSION 3.13)

add_executable(
  UTEST_JsonMembersPicker
  UTEST_JsonMembersPicker.cpp
  ${IADI_JSON_PARSER_SRC}
)

target_include_directories(
  UTEST_JsonMembersPicker
  PRIVATE ${GENERAL_MOCKS_ROOT}/log
  PRIVATE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
  UTEST_JsonMembersPicker
  GTest::gtest_main GTest::gmock
)

include(GoogleTest)

gtest_add_tests(
  TARGET UTEST_JsonMembersPicker
  TEST_SUFFIX .noArgs
  TEST_LIST noArgsTests
)

set_tests_properties(${noArgsTests} PROPERTIES TIMEOUT 600)

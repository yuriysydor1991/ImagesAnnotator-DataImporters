cmake_minimum_required(VERSION 3.13)

# The library is expected to log into the real logger instance handed over by
# its user, so the test links the produced shared library itself and drives it
# through the public LibraryFacade only - the very shape a consuming
# application sees. The RealDefaultLogger source below is the test own copy of
# the application side logger the instances are created from.
add_executable(
  CTEST_LibraryRealLogger
  CTEST_LibraryRealLogger.cpp
  ${CMAKE_SOURCE_DIR}/src/log/default-logger/real-default-logger/RealDefaultLogger.cpp
)

target_compile_definitions(
  CTEST_LibraryRealLogger
  PRIVATE CTEST_LibraryRealLogger_DATA_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
)

target_include_directories(
  CTEST_LibraryRealLogger
  PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${IADI_PUBLIC_INCLUDE_DIR}
)

target_link_libraries(
  CTEST_LibraryRealLogger
  ${PROJECT_LIBRARY_NAME}
  GTest::gtest_main GTest::gmock
)

include(GoogleTest)

gtest_add_tests(
  TARGET CTEST_LibraryRealLogger
  TEST_SUFFIX .noArgs
  TEST_LIST noArgsTests
)

set_tests_properties(${noArgsTests} PROPERTIES TIMEOUT 600)

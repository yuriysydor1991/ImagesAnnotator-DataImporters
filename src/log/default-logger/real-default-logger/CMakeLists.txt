cmake_minimum_required(VERSION 3.13)

add_library(
  TemplateProjectRealDefaultLoggerObj OBJECT
  RealDefaultLogger.cpp
)

target_include_directories(
  TemplateProjectRealDefaultLoggerObj
  PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

# The objects end up inside a shared library, so they must be relocatable and
# they must follow its hidden visibility: the logger is an implementation
# detail and its symbols would otherwise clash with the identically named ones
# of any other library built from this template.
set_target_properties(
  TemplateProjectRealDefaultLoggerObj
  PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
)

# The library code logs through the src/log macros, which resolve to this
# logger. Without this link the produced shared object would ship unresolved
# default_logger::RealDefaultLogger symbols and every consumer would fail to
# link against it. PRIVATE because the logger is an implementation detail: no
# installable public header mentions it.
target_link_libraries(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE TemplateProjectRealDefaultLoggerObj
)

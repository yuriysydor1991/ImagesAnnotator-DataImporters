cmake_minimum_required(VERSION 3.13)

# the plain text dataset layout importer

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    PlainTxtFolder2DBImporter.cpp
)

add_subdirectory(tests)

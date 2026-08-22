cmake_minimum_required(VERSION 3.13)

# the Create ML object detection dataset layout importer

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    CreateMLFolder2DBImporter.cpp
)

add_subdirectory(tests)

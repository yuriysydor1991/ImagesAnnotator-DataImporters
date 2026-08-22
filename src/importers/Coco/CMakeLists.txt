cmake_minimum_required(VERSION 3.13)

# the COCO object detection dataset layout importer

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    CocoFolder2DBImporter.cpp
)

add_subdirectory(tests)

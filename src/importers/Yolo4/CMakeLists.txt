cmake_minimum_required(VERSION 3.13)

# the YOLO v4 dataset layout importer

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    Yolo4Folder2DBImporter.cpp
)

add_subdirectory(tests)

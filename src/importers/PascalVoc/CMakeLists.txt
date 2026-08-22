cmake_minimum_required(VERSION 3.13)

# the Pascal VOC dataset layout importer

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    PascalVocFolder2DBImporter.cpp
)

add_subdirectory(tests)

cmake_minimum_required(VERSION 3.13)

# the three Ultralytics YOLO dataset layout importers, which share both the
# descriptor reader and the directory walk and differ in one label file line
# only

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    UltralyticsDataYaml.cpp
    UltralyticsFolder2DBImporter.cpp
    UltralyticsDetectFolder2DBImporter.cpp
    UltralyticsObbFolder2DBImporter.cpp
    UltralyticsSegmentFolder2DBImporter.cpp
)

add_subdirectory(tests)

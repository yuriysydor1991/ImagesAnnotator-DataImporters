cmake_minimum_required(VERSION 3.13)

# the JSON document reader, which the COCO and the Create ML layouts are read
# through

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    JsonValue.cpp
    JsonParser.cpp
)

add_subdirectory(tests)

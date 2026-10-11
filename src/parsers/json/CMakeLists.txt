cmake_minimum_required(VERSION 3.13)

# the JSON document reader, which the COCO and the Create ML layouts are read
# through, and the picker of the members of a document read a piece at a time,
# which the darknetxx ones are read through

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    JsonValue.cpp
    JsonParser.cpp
    JsonMembersPicker.cpp
)

add_subdirectory(tests)

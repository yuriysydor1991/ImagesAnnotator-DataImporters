cmake_minimum_required(VERSION 3.13)

# the XML document reader, which the Pascal VOC layout is read through

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    XmlNode.cpp
    XmlParser.cpp
)

add_subdirectory(tests)

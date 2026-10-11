cmake_minimum_required(VERSION 3.13)

# the darknetxx layouts importers: the dxxwjz2p and the dxxwjz3p training
# projects and the training directories, the YOLO v4 one with the dxxwjz2 or
# the dxxwjz3 network to train, with the reader of the members of their gzip
# compressed documents

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    DxxwjzFileReader.cpp
    Dxxwjz2pProject2DBImporter.cpp
    Dxxwjz3pProject2DBImporter.cpp
    Dxxwjz2Folder2DBImporter.cpp
    Dxxwjz3Folder2DBImporter.cpp
)

add_subdirectory(tests)

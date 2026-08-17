cmake_minimum_required(VERSION 3.13)

# One sub-directory per dataset layout, each with its own sources and its own
# unit tests. ImportersAliases.h and the Folder2DBImporter base stay here,
# since every one of them shares both.

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    Folder2DBImporter.cpp
)

add_subdirectory(PlainTxt)
add_subdirectory(Yolo4)
add_subdirectory(PyTorch)

add_subdirectory(tests)

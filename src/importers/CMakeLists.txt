cmake_minimum_required(VERSION 3.13)

# One sub-directory per dataset layout family, each with its own sources and
# its own unit tests. ImportersAliases.h and the Folder2DBImporter base stay
# here, since every one of them shares both, and so does the
# JsonDescriptor2DBImporter half the two JSON described layouts share.

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    Folder2DBImporter.cpp
    JsonDescriptor2DBImporter.cpp
)

add_subdirectory(PlainTxt)
add_subdirectory(Yolo4)
add_subdirectory(PyTorch)
add_subdirectory(Ultralytics)
add_subdirectory(Coco)
add_subdirectory(PascalVoc)
add_subdirectory(CreateML)

add_subdirectory(tests)

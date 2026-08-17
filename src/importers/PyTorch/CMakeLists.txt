cmake_minimum_required(VERSION 3.13)

# the PyTorch Vision dataset layout importer

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    PyTorchVisionFolder2DBImporter.cpp
)

add_subdirectory(tests)

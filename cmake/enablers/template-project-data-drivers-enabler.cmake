cmake_minimum_required(VERSION 3.13)

# The importers build their image records and fill the annotations database
# through the ImagesAnnotatorDataDrivers library, so unlike the optional
# third party enablers of the template this dependency is mandatory.
#
# This module only makes the dependency AVAILABLE. It runs before the library
# target exists, so that the unit and component test executables declared
# inside src/ may already link against it. The linking against the library
# target itself is performed later by the companion
# template-project-data-drivers-linker module.

set(
  IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE "ImagesAnnotatorDataDrivers-0.12"
  CACHE STRING
  "The CMake package name of the ImagesAnnotator data drivers library to build against"
)

# No version is requested on purpose: the 0.12.0 release of the data drivers
# library installs its package Config file without the companion
# <package>ConfigVersion.cmake, so a versioned find_package() would never
# match it. The package name already carries the major and the minor version.
find_package(${IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE} REQUIRED CONFIG)

set(
  IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET
  "${IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE}::${IMAGES_ANNOTATOR_DATA_DRIVERS_PACKAGE}"
)

message(STATUS "ImagesAnnotator data drivers target: ${IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET}")

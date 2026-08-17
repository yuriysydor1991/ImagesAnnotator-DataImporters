cmake_minimum_required(VERSION 3.13)

# The library decodes no image format of its own and asks its consumer for an
# IImageSizeFacility. This module makes OpenCV available so that it can also
# ship one itself, for consumers that have no imaging stack to lend.
#
# Taken from the appOpenCV branch of the project template, with one deliberate
# difference: there the dependency is mandatory and the module ends in a
# find_package(OpenCV REQUIRED ...), which fails the configure on a system
# without OpenCV. Here it is optional. A missing OpenCV only means the library
# is built without its own image size reader, and the layouts which need one -
# every layout storing normalised coordinates and the PyTorch Vision one -
# keep asking the consumer for it. The component list is the template's reduced
# to what the reader actually calls: it only decodes a header, so core and
# imgcodecs, and neither the objdetect nor the imgproc of the template.
#
# This module only makes the dependency AVAILABLE. It runs before the library
# target exists, so that the unit test executables declared inside src/ may
# already link against it. The linking against the library target itself is
# performed later by the companion template-project-OpenCV-linker module.
option(
  ENABLE_OPENCV
  "Set to ON to probe the system OpenCV and build the library's own image size reader with it"
  ON
)

# The one variable the rest of the build asks: src/sizers adds the reader
# source only when it is ON, and the linker module below links OpenCV and
# defines IADI_WITH_OPENCV for the compilation.
set(IADI_WITH_OPENCV OFF)

if (NOT ENABLE_OPENCV)
  message(STATUS
    "OpenCV integration is OFF - the library ships no image size reader of "
    "its own and every consumer has to supply one")
  return()
endif()

set(
  TEMPLATE_APP_OPENCV_COMPONENTS
  "core;imgcodecs"
  CACHE STRING
  "Semicolon-separated list of OpenCV components required by the project"
)

find_package(OpenCV QUIET COMPONENTS ${TEMPLATE_APP_OPENCV_COMPONENTS})

if (NOT OpenCV_FOUND)
  message(STATUS
    "OpenCV was not found - the library ships no image size reader of its "
    "own; the YOLO v4 and the PyTorch Vision imports will keep asking their "
    "consumer for one. Install the OpenCV development packages, or point the "
    "configure at them with -DOpenCV_DIR=<dir>, to build it.")
  return()
endif()

set(IADI_WITH_OPENCV ON)

message(STATUS "OpenCV ${OpenCV_VERSION} found at ${OpenCV_INCLUDE_DIRS}")

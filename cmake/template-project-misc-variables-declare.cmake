cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ON by default: the public API namespace carries the major and the minor
# version (ImagesAnnotatorDataImporters013), so the installable names track the
# same pair. Two minor versions then install completely side by side - their
# binaries, their header directories and their CMake packages all differ. This
# is also what the ImagesAnnotatorDataDrivers library this project fills its
# databases through does, so both halves of the pair are named alike.
option(
  LIB_INCLUDE_MINOR_IN_NAME
  "Append .<minor> to the installable library name (binary, headers subdir, CMake package dir)"
  ON
)

option(
  LIB_INCLUDE_MICRO_IN_NAME
  "Append .<micro> after the minor segment to the installable library name. Implies LIB_INCLUDE_MINOR_IN_NAME."
  OFF
)

set(
  LIB_NAME_SUFFIX ""
  CACHE STRING
  "Optional trailing suffix appended to the installable library name (e.g. '-dev')."
)

set(_lib_name "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION_MAJOR}")

if(LIB_INCLUDE_MICRO_IN_NAME AND NOT LIB_INCLUDE_MINOR_IN_NAME)
  message(WARNING
    "LIB_INCLUDE_MICRO_IN_NAME=ON requires LIB_INCLUDE_MINOR_IN_NAME=ON - "
    "enabling LIB_INCLUDE_MINOR_IN_NAME implicitly."
  )
  set(LIB_INCLUDE_MINOR_IN_NAME ON CACHE BOOL "" FORCE)
endif()

if(LIB_INCLUDE_MINOR_IN_NAME)
  set(_lib_name "${_lib_name}.${CMAKE_PROJECT_VERSION_MINOR}")
endif()

if(LIB_INCLUDE_MICRO_IN_NAME)
  set(_lib_name "${_lib_name}.${CMAKE_PROJECT_VERSION_PATCH}")
endif()

if(LIB_NAME_SUFFIX)
  set(_lib_name "${_lib_name}${LIB_NAME_SUFFIX}")
endif()

set(
  PROJECT_LIBRARY_NAME "${_lib_name}"
  CACHE STRING "Project main library name and target"
  FORCE
)

unset(_lib_name)

message(STATUS "PROJECT_LIBRARY_NAME: ${PROJECT_LIBRARY_NAME}")

# The directory holding the installable public headers. Unlike in the project
# template these are written by hand rather than generated, since the public
# namespace of this library is a fixed ImagesAnnotatorDataImporters013 instead
# of one derived from the project name. The variable is declared here and not
# in the src/lib subdirectory, because the analyzer targets are created before
# that subdirectory is added and have to reach those headers too.
set(
  IADI_PUBLIC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/lib/facade/public"
  CACHE PATH "The library public headers include directory"
  FORCE
)

set(
  PROJECT_MAINTAINER "Your Name"
  CACHE STRING "Project maintainer name (used for the DEB package)"
)

set(
  PROJECT_MAINTAINER_EMAIL "yourname@your.email.org"
  CACHE STRING "Project maintainer email (used for the DEB package)"
)

set(
  MAX_LOG_LEVEL "3"
  CACHE STRING
  "Sets the maximum severity of the compiled log messages. Error=0. Trace=5. By default Info=3"
)

set(
  DEFAULT_LOG_FILE_PATH ""
  CACHE STRING
  "Sets the default log file path"
)

option(
  ENABLE_LOGS_MICROSECONDS_TIME
  "Set to OFF to disable the microseconds fields included into the log messages"
  ON
)

string(TIMESTAMP PROJECT_CONFIGURE_DATE "%Y-%m-%d %H:%M:%S")

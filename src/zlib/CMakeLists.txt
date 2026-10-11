cmake_minimum_required(VERSION 3.13)

# The zlib compression controller of the cpp-app-template appZlib branch, as
# the darknetxx adapted it for its gzip compressed dxxwjz files: the in memory
# and the gzip file compression and the gzip files streams, which read and
# write the files of the hundreds of megabytes a chunk at a time.

target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    GzipFileStreams.cpp
    ZlibController.cpp
)

add_subdirectory(tests)

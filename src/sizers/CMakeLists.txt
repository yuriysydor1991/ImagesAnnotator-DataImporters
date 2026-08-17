cmake_minimum_required(VERSION 3.13)

# The factory is compiled into every build of the library, with OpenCV and
# without: it is what answers "did this build get an image size reader of its
# own?" at the run time, so that no caller has to repeat the build time
# condition.
target_sources(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE
    ImageSizeFactory.cpp
)

# The implementation behind it is added only when the OpenCV enabler found the
# library. Without it the factory compiles its nullptr branch and the imports
# which need a measurement keep asking their consumer for a reader.
if (IADI_WITH_OPENCV)
  target_sources(
    ${PROJECT_LIBRARY_NAME}
    PRIVATE
      OpenCVImageSizer.cpp
  )

  target_include_directories(
    ${PROJECT_LIBRARY_NAME}
    PRIVATE ${OpenCV_INCLUDE_DIRS}
  )
endif()

add_subdirectory(tests)

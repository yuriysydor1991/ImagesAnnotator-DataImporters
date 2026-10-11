cmake_minimum_required(VERSION 3.13)

# Companion of the template-project-zlib-enabler module: links the already
# made available zlib against the project library target, which only exists
# once src/ has been added.

# PRIVATE on purpose: no installable public header of the library exposes a
# zlib type, so the dependency must not leak into the exported interface.
target_link_libraries(
  ${PROJECT_LIBRARY_NAME}
  PRIVATE ZLIB::ZLIB
)

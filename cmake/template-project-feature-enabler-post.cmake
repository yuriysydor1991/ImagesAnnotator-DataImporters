cmake_minimum_required(VERSION 3.13)

include(template-project-dockerers)

# The mandatory dependencies were made available by the pre-src enabler pass,
# here they only get linked against the now existing library target.
include(template-project-data-drivers-linker)
include(template-project-zlib-linker)
include(template-project-OpenCV-linker)

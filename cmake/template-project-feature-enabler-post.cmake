cmake_minimum_required(VERSION 3.13)

include(template-project-dockerers)

# The mandatory dependency was made available by the pre-src enabler pass,
# here it only gets linked against the now existing library target.
include(template-project-data-drivers-linker)
include(template-project-OpenCV-linker)

cmake_minimum_required(VERSION 3.13)

# Both helpers of this library are header only, so nothing is registered
# against the library target here. The unit tests of the sub-directory below
# are what keeps them covered.

add_subdirectory(tests)

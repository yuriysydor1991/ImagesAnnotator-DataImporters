cmake_minimum_required(VERSION 3.13)

# One sub-directory per document format a dataset descriptor is written in.
# Neither reader knows anything of the layouts which are read through it, which
# is why both live here instead of under the importers and why both are tested
# on their own documents.

add_subdirectory(json)
add_subdirectory(xml)

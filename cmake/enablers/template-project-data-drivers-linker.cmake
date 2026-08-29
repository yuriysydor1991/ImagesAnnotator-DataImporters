cmake_minimum_required(VERSION 3.13)

# Companion of the template-project-data-drivers-enabler module: links the
# already made available data drivers library against the project library
# target, which only exists once src/ has been added.

# PUBLIC on purpose, unlike every other dependency of this library: the
# installable IADataImportersContext.h and IImageSizeFacility.h headers name the
# ImagesAnnotatorDataDrivers database and record types, so a consumer needs
# both its include path and its shared object.
target_link_libraries(
  ${PROJECT_LIBRARY_NAME}
  PUBLIC ${IMAGES_ANNOTATOR_DATA_DRIVERS_TARGET}
)

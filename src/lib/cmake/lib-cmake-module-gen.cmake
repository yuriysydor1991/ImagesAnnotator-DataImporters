include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PROJECT_MODULE_TARGETS_FILE "${PROJECT_LIBRARY_NAME}Targets.cmake")
set(PROJECT_MODULE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_LIBRARY_NAME}")

install(
  TARGETS ${PROJECT_LIBRARY_NAME}
  EXPORT ${PROJECT_LIBRARY_NAME}
  LIBRARY
    DESTINATION lib
    COMPONENT Runtime
    NAMELINK_COMPONENT Development
)

# Export targets (so downstream can use find_package)
install(EXPORT ${PROJECT_LIBRARY_NAME}
  FILE ${PROJECT_MODULE_TARGETS_FILE}
  NAMESPACE ${PROJECT_LIBRARY_NAME}::
  DESTINATION ${PROJECT_MODULE_INSTALL_DIR}
  COMPONENT Development
)

# Generate a ConfigVersion.cmake. The name matters: find_package() only looks
# for <package>ConfigVersion.cmake, so a differently named file leaves the
# package version invisible and every versioned find_package() call fails.
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_LIBRARY_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Generate a Config.cmake. It is the file that pulls the public dependencies
# in before including the exported targets, so it must not be replaced by the
# raw targets file.
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ImportersLibraryConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_LIBRARY_NAME}Config.cmake"
  INSTALL_DESTINATION ${PROJECT_MODULE_INSTALL_DIR}
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_LIBRARY_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_LIBRARY_NAME}ConfigVersion.cmake"
  DESTINATION ${PROJECT_MODULE_INSTALL_DIR}
  COMPONENT Development
)

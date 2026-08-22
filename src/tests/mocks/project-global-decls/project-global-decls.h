#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_GLOBAL_DECLS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_GLOBAL_DECLS_H

#include <string>

/**
 * @brief The unit tests stand-in for the CMake generated
 * project-global-decls.h header.
 *
 * Unit tests compile single translation units straight from the source tree,
 * without the configured build directory on their include path, so they pick
 * this fixed copy up instead.
 */
namespace project_decls
{

/// @brief Global project' library name
const std::string PROJECT_NAME = "ImagesAnnotatorDataImporters-0.13";
/// @brief Global project' build version
const std::string PROJECT_BUILD_VERSION = "0.13.0";
/// @brief Global project' configure git commit hash
const std::string PROJECT_BUILD_COMMIT =
    "0000000000000000000000000000000000000000";
/// @brief Global project' configure date
const std::string PROJECT_CONFIGURE_DATE = "2025-01-01 00:00:00";

}  // namespace project_decls

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_GLOBAL_DECLS_H

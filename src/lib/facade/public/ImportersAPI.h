/**
 * The "Simplified BSD License"
 *
 * Copyright (c) 2026, Yurii Sydor (yuriysydor1991@gmail.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMPORTERSAPI_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMPORTERSAPI_H

/**
 * @brief Marks the entities that make up the installable library interface.
 *
 * The library is compiled with the hidden symbol visibility, so that its
 * implementation classes - iadi0impl::LibFactory,
 * default_logger::DefaultLogger, the project_decls constants and the
 * iannotator::importers importers - stay private to the shared object. That is
 * not a size optimisation: the ImagesAnnotatorDataDrivers library this one
 * links against is built from the very same project template and carries its
 * own default_logger and project_decls definitions. Were both sets exported,
 * the dynamic linker would bind one library's calls to the other library's
 * definitions.
 *
 * Hiding alone does not cover the factory: the std::make_shared instantiations
 * name their class in the mangled name and stay weak and exported whatever the
 * visibility. That is why this implementation namespace is iadi0impl and not
 * the lib0impl the project template - and the data drivers library with it -
 * uses.
 *
 * Current file is a target for the library header installation.
 */
#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef IMAGES_ANNOTATOR_DATA_IMPORTERS_BUILDING
#define IADI_API __declspec(dllexport)
#else
#define IADI_API __declspec(dllimport)
#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_BUILDING
#else
#define IADI_API __attribute__((visibility("default")))
#endif  // _WIN32

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_IMPORTERSAPI_H

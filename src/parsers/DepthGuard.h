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

#ifndef IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DEPTHGUARD_CLASS_H
#define IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DEPTHGUARD_CLASS_H

#include <cstddef>

namespace iannotator::importers::parsers
{

/**
 * @brief Counts one nesting level of a document parse for as long as that
 * level runs.
 *
 * Both readers of this component descend recursively, so each of them walks
 * the stack down together with the document it is handed, and a file made of
 * nothing but opening brackets would walk it away. The counter this guard
 * keeps is what the readers refuse such a document by, and it has to be given
 * back whatever the way a nested parse is left - the return of the read value
 * as much as the return of a malformed one.
 */
class DepthGuard
{
 public:
  explicit DepthGuard(std::size_t& ndepth) : depth{ndepth} { ++depth; }
  ~DepthGuard() { --depth; }

  DepthGuard(const DepthGuard&) = delete;
  DepthGuard(DepthGuard&&) = delete;
  DepthGuard& operator=(const DepthGuard&) = delete;
  DepthGuard& operator=(DepthGuard&&) = delete;

 private:
  std::size_t& depth;
};

}  // namespace iannotator::importers::parsers

#endif  // IMAGES_ANNOTATOR_DATA_IMPORTERS_PROJECT_DEPTHGUARD_CLASS_H

// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>
#include <string_view>

namespace Lingmo::Shell {

// Case-insensitive search: each character compared by its simple Unicode case
// folding, so "É" finds "é" and "Σ" finds "ς".
bool containsIgnoringCase(std::string_view text, std::string_view needle);

// How well `needle` names `text`, 0 when it does not. Higher is a better
// match, and the bands never overlap, so a caller can rank on the number
// alone: an exact name beats a prefix, a prefix beats a word start, and a
// word start beats the characters merely appearing in order.
int fuzzyMatchScore(std::string_view text, std::string_view needle);

// Unicode lower case, not only ASCII.
std::string toLower(std::string_view text);

} // namespace Lingmo::Shell


// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "display/display_types.h"
#include <set>
#include <vector>

namespace Lingmo::Display {

class DisplayIdentity
{
public:
    void seed(const std::vector<DisplaySnapshot> &displays) { m_known = displays; }

    void rememberPersistentKeys(const std::vector<std::string> &keys);
    void assign(std::vector<DisplaySnapshot> &displays);

private:
    std::vector<DisplaySnapshot> m_known;
    std::set<std::string> m_ambiguous;
};

} // namespace Lingmo::Display

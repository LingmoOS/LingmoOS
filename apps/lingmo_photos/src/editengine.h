// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <string>

namespace Photos {

struct EditParams {
    double brightness = 0;
    double contrast = 0;
    double saturation = 0;
    double temperature = 0;
    int rotation = 0;
};

// Writes an edited copy without modifying the source file.
bool saveEditedCopy(const std::string& source, const std::string& target, const EditParams& params, std::string* error = nullptr);

} // namespace Photos

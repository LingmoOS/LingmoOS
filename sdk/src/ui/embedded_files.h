// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

namespace Lingmo::ui {

// Files built into the library, such as the glyph icons under data/icons.
struct EmbeddedFile
{
    const char* name;
    const unsigned char* data;
    size_t size;
};

const EmbeddedFile* findEmbeddedFile(const char* name);

} // namespace Lingmo::ui

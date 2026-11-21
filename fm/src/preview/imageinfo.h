// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace fm {

// What Quick Look needs of an image before decoding it, from its header: the
// pixel size after the EXIF orientation, whether it moves, whether it is
// drawn from vectors.
struct ImageInfo
{
    // The file is an image the toolkit decodes.
    bool readable = false;
    // 0 when the header does not say.
    int width = 0;
    int height = 0;
    bool animated = false;
    bool vector = false;
};

ImageInfo readImageInfo(const std::string& path, const std::string& mimeType);

} // namespace fm

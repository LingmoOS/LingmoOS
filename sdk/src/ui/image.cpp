// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/image.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"

namespace Lingmo::ui {

Image::Image(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_pixels(size_t(width) * size_t(height))
{
}

bool Image::savePng(const std::string& path) const
{
    if (isNull())
        return false;
    // 0xAARRGGBB words are BGRA bytes on little-endian machines.
    const SkImageInfo info = SkImageInfo::Make(m_width, m_height, kBGRA_8888_SkColorType,
                                               kUnpremul_SkAlphaType);
    const SkPixmap pixmap(info, m_pixels.data(), size_t(m_width) * 4);
    SkFILEWStream stream(path.c_str());
    return stream.isValid() && SkPngEncoder::Encode(&stream, pixmap, {});
}

} // namespace Lingmo::ui

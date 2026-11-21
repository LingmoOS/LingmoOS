// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "modules/skcms/skcms.h"

namespace Lingmo::ui {

// The colours a decoder's RGB pixels are in, for turning them into the sRGB
// everything is drawn in.
class SourceProfile
{
public:
    // Null when the bytes are no RGB profile the transform can read, or one
    // that is sRGB already.
    static std::unique_ptr<SourceProfile> fromIcc(const void* data, size_t size);

    // From the ITU-T H.273 code points with the primaries' and white point's
    // chromaticities; null for the sRGB primaries, unspecified ones, and the
    // high dynamic range transfers, which need tone mapping.
    static std::unique_ptr<SourceProfile> fromCicp(int transfer, const float primaries[8]);

    // The ICC profile of a WebP file's ICCP chunk.
    static std::unique_ptr<SourceProfile> fromWebp(const std::string& data);

    SourceProfile(const SourceProfile&) = delete;
    SourceProfile& operator=(const SourceProfile&) = delete;

    // Turns 8-bit RGBA or BGRA pixels into sRGB in place, premultiplied or
    // straight as they are.
    void toSrgb(uint8_t* pixels, int width, int height, size_t stride, bool bgra, bool premultiplied) const;

private:
    SourceProfile() = default;

    // What the profile's tables point into.
    std::vector<uint8_t> m_bytes;
    skcms_ICCProfile m_profile;
};

} // namespace Lingmo::ui

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_profile.h"

#include <webp/demux.h>

namespace Lingmo::ui {

std::unique_ptr<SourceProfile> SourceProfile::fromIcc(const void* data, size_t size)
{
    if (!data || size == 0)
        return nullptr;
    std::unique_ptr<SourceProfile> profile(new SourceProfile);
    const auto* bytes = static_cast<const uint8_t*>(data);
    profile->m_bytes.assign(bytes, bytes + size);
    if (!skcms_Parse(profile->m_bytes.data(), profile->m_bytes.size(), &profile->m_profile)
        || profile->m_profile.data_color_space != skcms_Signature_RGB
        || skcms_ApproximatelyEqualProfiles(&profile->m_profile, skcms_sRGB_profile()))
        return nullptr;
    return profile;
}

std::unique_ptr<SourceProfile> SourceProfile::fromCicp(int transfer, const float primaries[8])
{
    skcms_TransferFunction curve;
    switch (transfer) {
    case 8:
        curve = {1, 1, 0, 0, 0, 0, 0};
        break;
    // BT.709, unspecified, BT.601, sRGB and the BT.2020 SDR curves: shown
    // with the sRGB curve, as displays show such pictures.
    case 1:
    case 2:
    case 6:
    case 13:
    case 14:
    case 15:
        curve = *skcms_sRGB_TransferFunction();
        break;
    default:
        return nullptr;
    }
    skcms_Matrix3x3 toXYZD50;
    if (!skcms_PrimariesToXYZD50(primaries[0], primaries[1], primaries[2], primaries[3], primaries[4],
                                 primaries[5], primaries[6], primaries[7], &toXYZD50))
        return nullptr;
    std::unique_ptr<SourceProfile> profile(new SourceProfile);
    skcms_Init(&profile->m_profile);
    skcms_SetTransferFunction(&profile->m_profile, &curve);
    skcms_SetXYZD50(&profile->m_profile, &toXYZD50);
    if (skcms_ApproximatelyEqualProfiles(&profile->m_profile, skcms_sRGB_profile()))
        return nullptr;
    return profile;
}

std::unique_ptr<SourceProfile> SourceProfile::fromWebp(const std::string& data)
{
    const WebPData webp = {reinterpret_cast<const uint8_t*>(data.data()), data.size()};
    std::unique_ptr<WebPDemuxer, decltype(&WebPDemuxDelete)> demuxer(WebPDemux(&webp), WebPDemuxDelete);
    if (!demuxer || !(WebPDemuxGetI(demuxer.get(), WEBP_FF_FORMAT_FLAGS) & ICCP_FLAG))
        return nullptr;
    WebPChunkIterator chunk;
    if (!WebPDemuxGetChunk(demuxer.get(), "ICCP", 1, &chunk))
        return nullptr;
    std::unique_ptr<SourceProfile> profile = fromIcc(chunk.chunk.bytes, chunk.chunk.size);
    WebPDemuxReleaseChunkIterator(&chunk);
    return profile;
}

void SourceProfile::toSrgb(uint8_t* pixels, int width, int height, size_t stride, bool bgra,
                           bool premultiplied) const
{
    const skcms_PixelFormat format = bgra ? skcms_PixelFormat_BGRA_8888 : skcms_PixelFormat_RGBA_8888;
    const skcms_AlphaFormat alpha = premultiplied ? skcms_AlphaFormat_PremulAsEncoded : skcms_AlphaFormat_Unpremul;
    if (stride == size_t(width) * 4) {
        skcms_Transform(pixels, format, alpha, &m_profile, pixels, format, alpha, skcms_sRGB_profile(),
                        size_t(width) * size_t(height));
        return;
    }
    for (int y = 0; y < height; ++y) {
        uint8_t* row = pixels + size_t(y) * stride;
        skcms_Transform(row, format, alpha, &m_profile, row, format, alpha, skcms_sRGB_profile(), size_t(width));
    }
}

} // namespace Lingmo::ui

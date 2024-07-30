/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include "format_p.h"

using namespace Prison;

struct format_map_t {
    ZXing::BarcodeFormat zxFormat;
    Prison::Format::BarcodeFormat format;
};

static constexpr const format_map_t format_map[] = {
    {ZXing::BarcodeFormat::None, Format::NoFormat},
    {ZXing::BarcodeFormat::Aztec, Format::Aztec},
    {ZXing::BarcodeFormat::Codabar, Format::Codabar},
    {ZXing::BarcodeFormat::Code39, Format::Code39},
    {ZXing::BarcodeFormat::Code93, Format::Code93},
    {ZXing::BarcodeFormat::Code128, Format::Code128},
    {ZXing::BarcodeFormat::DataBar, Format::DataBar},
    {ZXing::BarcodeFormat::DataBarExpanded, Format::DataBarExpanded},
    {ZXing::BarcodeFormat::DataMatrix, Format::DataMatrix},
    {ZXing::BarcodeFormat::EAN8, Format::EAN8},
    {ZXing::BarcodeFormat::EAN13, Format::EAN13},
    {ZXing::BarcodeFormat::ITF, Format::ITF},
    {ZXing::BarcodeFormat::MaxiCode, Format::MaxiCode},
    {ZXing::BarcodeFormat::PDF417, Format::PDF417},
    {ZXing::BarcodeFormat::QRCode, Format::QRCode},
    {ZXing::BarcodeFormat::UPCA, Format::UPCA},
    {ZXing::BarcodeFormat::UPCE, Format::UPCE},
};

ZXing::BarcodeFormats Format::toZXing(Format::BarcodeFormats formats)
{
    ZXing::BarcodeFormats f;
    for (auto m : format_map) {
        if (m.format & formats) {
            f |= m.zxFormat;
        }
    }
    return f;
}

Format::BarcodeFormat Format::toFormat(ZXing::BarcodeFormat format)
{
    const auto it = std::find_if(std::begin(format_map), std::end(format_map), [format](auto m) {
        return m.zxFormat == format;
    });

    return it != std::end(format_map) ? (*it).format : Format::NoFormat;
}

#include "moc_format.cpp"

/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "pdf417barcode_p.h"
#include "zxingutil_p.h"

#include <ZXing/BitMatrix.h>
#include <ZXing/MultiFormatWriter.h>

#include <stdexcept>

using namespace Prison;

Pdf417Barcode::Pdf417Barcode()
    : AbstractBarcodePrivate(Barcode::TwoDimensions)
{
}

QImage Pdf417Barcode::paintImage()
{
    try {
        ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::PDF417);
        // ISO/IEC 15438:2006(E) §5.8.3 Quiet Zone
        writer.setMargin(2);
        if (m_data.userType() == QMetaType::QByteArray) {
            writer.setEncoding(ZXing::CharacterSet::BINARY);
        }
        // aspect ratio 4 is hard-coded in ZXing
        const auto matrix = writer.encode(ZXingUtil::toStdWString(m_data), 4, 1);
        return ZXingUtil::toImage(matrix, m_foreground, m_background);
    } catch (const std::invalid_argument &e) {
    }; // input too large
    return {};
}

QSizeF Pdf417Barcode::preferredSize(qreal devicePixelRatio) const
{
    return m_cache.size() * (devicePixelRatio < 2 ? 2 : 1);
}

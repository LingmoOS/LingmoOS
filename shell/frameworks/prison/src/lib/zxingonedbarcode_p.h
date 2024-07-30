/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_ZXINGONEDBARCODE_H
#define PRISON_ZXINGONEDBARCODE_H

#include "abstractbarcode_p.h"
#include "zxingutil_p.h"

#include <ZXing/BitMatrix.h>
#include <ZXing/MultiFormatWriter.h>

#include <stdexcept>

namespace Prison
{
/** Generic support for ZXing 1D barcodes. */
template<ZXing::BarcodeFormat Format>
class ZXingOneDBarcode : public AbstractBarcodePrivate
{
public:
    explicit inline ZXingOneDBarcode()
        : AbstractBarcodePrivate(Barcode::OneDimension)
    {
    }

protected:
    inline QImage paintImage() override
    {
        try {
            ZXing::MultiFormatWriter writer(Format);
            const auto matrix = writer.encode(ZXingUtil::toStdWString(m_data), 1, 1);
            return ZXingUtil::toImage(matrix, m_foreground, m_background);
        } catch (const std::invalid_argument &e) {
        }; // input too large or incompatible
        return {};
    }
};

}

#endif // PRISON_PDF417BARCODE_H

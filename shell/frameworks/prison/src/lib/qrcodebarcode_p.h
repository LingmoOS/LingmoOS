/*
    SPDX-FileCopyrightText: 2010-2014 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_QRCODEBARCODE_H
#define PRISON_QRCODEBARCODE_H

#include "abstractbarcode_p.h"

namespace Prison
{
/**
 * QRCode Barcode generator ; uses libqrencode to do the actual encoding
 * of the barcode.
 */
class QRCodeBarcode : public AbstractBarcodePrivate
{
public:
    /**
     * creates a QRCode generator
     */
    QRCodeBarcode();
    ~QRCodeBarcode() override;
    /**
     * This is the function doing the actual work in generating the barcode
     * @return QImage containing a QRCode, trying to approximate the requested sizes
     */
    QImage paintImage() override;
};
} // namespace

#endif // PRISON_QRCODEBARCODE_H

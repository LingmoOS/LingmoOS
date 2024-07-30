/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_PDF417BARCODE_H
#define PRISON_PDF417BARCODE_H

#include "abstractbarcode_p.h"

namespace Prison
{
/** PDF417 barcode.
 *  @see https://en.wikipedia.org/wiki/PDF417
 *  @see ISO/IEC 15438
 */
class Pdf417Barcode : public AbstractBarcodePrivate
{
public:
    explicit Pdf417Barcode();

protected:
    QImage paintImage() override;
    QSizeF preferredSize(qreal devicePixelRatio) const override;
};

}

#endif // PRISON_PDF417BARCODE_H

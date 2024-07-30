/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_CODE128BARCODE_H
#define PRISON_CODE128BARCODE_H

#include "abstractbarcode_p.h"

class Code128BarcodeTest;

namespace Prison
{
class BitVector;

/** Code 128 barcode
 *  @see https://en.wikipedia.org/wiki/Code_128
 */
class Code128Barcode : public AbstractBarcodePrivate
{
public:
    Code128Barcode();
    ~Code128Barcode() override;

protected:
    QImage paintImage() override;

private:
    friend class ::Code128BarcodeTest;
    BitVector encode(const QByteArray &data) const;
};

}

#endif // PRISON_CODE128BARCODE_H

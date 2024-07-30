/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_AZTECBARCODE_H
#define PRISON_AZTECBARCODE_H

#include "abstractbarcode_p.h"

class AztecBarcodeTest;

namespace Prison
{
class BitVector;

/** Aztec code generator. */
class AztecBarcode : public AbstractBarcodePrivate
{
public:
    AztecBarcode();
    ~AztecBarcode() override;

protected:
    QImage paintImage() override;

private:
    friend class ::AztecBarcodeTest;

    BitVector aztecEncode(const QByteArray &data) const;
    BitVector bitStuffAndPad(const BitVector &input, int codeWordSize) const;

    void paintFullGrid(QImage *img) const;
    void paintFullData(QImage *img, const BitVector &data, int layerCount) const;
    void paintFullModeMessage(QImage *img, const BitVector &modeData) const;
    QImage cropAndScaleFull(QImage *img, int layerCount);

    void paintCompactGrid(QImage *img) const;
    void paintCompactData(QImage *img, const BitVector &data, int layerCount) const;
    void paintCompactModeMessage(QImage *img, const BitVector &modeData) const;
    QImage cropAndScaleCompact(QImage *img, int layerCount);
};

}

#endif // PRISON_AZTECCODE_H

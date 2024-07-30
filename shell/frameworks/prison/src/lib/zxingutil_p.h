/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_ZXINGUTILS_P_H
#define PRISON_ZXINGUTILS_P_H

#include <string>

namespace ZXing
{
class BitMatrix;
}

class QColor;
class QImage;
class QVariant;

namespace Prison
{
/** Utilities for interfacing with ZXing. */
namespace ZXingUtil
{
/** Convert barcode content (string or byte array) to the input
 *  format expected by ZXing.
 */
std::wstring toStdWString(const QVariant &data);

/** Convert the bitmatrix output of ZXing to a QImage. */
QImage toImage(const ZXing::BitMatrix &matrix, const QColor &foreground, const QColor &background);
}

}

#endif

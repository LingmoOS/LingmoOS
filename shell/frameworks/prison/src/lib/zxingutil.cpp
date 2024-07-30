/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include "zxingutil_p.h"

#include <QImage>
#include <QVariant>

#include <ZXing/BitMatrix.h>

using namespace Prison;

std::wstring ZXingUtil::toStdWString(const QVariant &data)
{
    if (data.userType() == QMetaType::QString) {
        return data.toString().toStdWString();
    }
    if (data.userType() == QMetaType::QByteArray) {
        const auto b = data.toByteArray();
        std::wstring ws;
        ws.reserve(b.size());
        // ensure we explicitly copy unsigned bytes here, ie. each byte ends up in the least significant byte of
        // the std::wstring. If we end up converting to a signed value inbetween, values > 127 end up negative and
        // will be wrongly represented in the std::wstring
        for (uint8_t c : b) {
            ws.push_back(c);
        }
        return ws;
    }

    return {};
}

QImage ZXingUtil::toImage(const ZXing::BitMatrix &matrix, const QColor &foreground, const QColor &background)
{
    QImage image(matrix.width(), matrix.height(), QImage::Format_ARGB32);
    for (int y = 0; y < matrix.height(); ++y) {
        for (int x = 0; x < matrix.width(); ++x) {
            image.setPixel(x, y, matrix.get(x, y) ? foreground.rgb() : background.rgb());
        }
    }
    return image;
}

/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_SCANRESULT_P_H
#define PRISON_SCANRESULT_P_H

#include "scanresult.h"

#include <QSharedData>
#include <QTransform>

#define ZX_USE_UTF8 1
#include <ZXing/Result.h>

namespace Prison
{

class ScanResultPrivate : public QSharedData
{
public:
    [[nodiscard]] static ScanResult fromZXingResult(const ZXing::Result &zxRes, const QTransform &transform = QTransform());

    QVariant content;
    QRect boundingRect;
    Format::BarcodeFormat format = Format::NoFormat;
};

}
#endif // PRISON_SCANRESULT_P_H

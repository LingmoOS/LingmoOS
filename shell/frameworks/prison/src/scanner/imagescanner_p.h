/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_IMAGESCANNER_P_H
#define PRISON_IMAGESCANNER_P_H

#include "format.h"

#define ZX_USE_UTF8 1
#include <ZXing/Result.h>

class QImage;

namespace Prison
{

namespace ImageScanner
{

[[nodiscard]] ZXing::Result readBarcode(const QImage &image, Format::BarcodeFormats formats);

}
}

#endif // PRISON_IMAGESCANNER_P_H

/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_FORMAT_P_H
#define PRISON_FORMAT_P_H

#include "format.h"

#include <ZXing/BarcodeFormat.h>

namespace Prison
{
namespace Format
{

ZXing::BarcodeFormats toZXing(BarcodeFormats formats);
BarcodeFormat toFormat(ZXing::BarcodeFormat format);

}
}

#endif // PRISON_FORMAT_H

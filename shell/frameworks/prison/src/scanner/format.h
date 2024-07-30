/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_FORMAT_H
#define PRISON_FORMAT_H

#include "prisonscanner_export.h"

#include <QFlag>
#include <QMetaType>

namespace Prison
{

/**
 * Barcode formats detectable by Prison::VideoScanner.
 *
 * @see Prison::ScanResult.
 * @since 5.94
 */
namespace Format
{
Q_NAMESPACE_EXPORT(PRISONSCANNER_EXPORT)
/** Barcode formats. */
enum BarcodeFormat {
    NoFormat = 0,
    Aztec = 1,
    Codabar = 2,
    Code39 = 4,
    Code93 = 8,
    Code128 = 16,
    DataBar = 32,
    DataBarExpanded = 64,
    DataMatrix = 128,
    EAN8 = 256,
    EAN13 = 512,
    ITF = 1024,
    MaxiCode = 2048,
    PDF417 = 4096,
    QRCode = 8192,
    UPCA = 16384,
    UPCE = 32768,
};
Q_ENUM_NS(BarcodeFormat)

Q_DECLARE_FLAGS(BarcodeFormats, BarcodeFormat)
Q_FLAG_NS(BarcodeFormats)
Q_DECLARE_OPERATORS_FOR_FLAGS(BarcodeFormats)
}
}

#endif // PRISON_FORMAT_H

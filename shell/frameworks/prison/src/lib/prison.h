/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_PRISON_H
#define PRISON_PRISON_H

#include "prison_export.h"

/**
 * @namespace Prison
 *
 * Provides classes and methods for generating barcodes.
 */
namespace Prison
{
/**
 * possible supported barcode types
 */
enum BarcodeType {
    /** QRCode 2d barcode */
    QRCode,
    /** DataMatrix 2d barcode */
    DataMatrix,
    /** Aztec 2d barcode */
    Aztec,
    /** Code39 barcode */
    Code39,
    /** Code93 barcode */
    Code93,
    /** Code 128 barcode */
    Code128,
    /** PDF417 barcode */
    PDF417,
    /** EAN13 barcode */
    EAN13,
};
}

#endif // PRISON_PRISON_H

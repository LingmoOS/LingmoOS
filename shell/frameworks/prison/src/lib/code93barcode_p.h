/*
    SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_CODE93BARCODE_H
#define PRISON_CODE93BARCODE_H

#include "abstractbarcode_p.h"

namespace Prison
{
/**
 * Code 93 Barcode generator
 */
class Code93Barcode : public Prison::AbstractBarcodePrivate
{
public:
    /**
     * creates a Code 93 generator
     */
    Code93Barcode();
    ~Code93Barcode() override;
    /**
     * This function generates the barcode
     * @return QImage containing a barcode, trying to approximate the requested sizes
     */
    QImage paintImage() override;
};
} // namespace

#endif // PRISON_CODE39BARCODE_H

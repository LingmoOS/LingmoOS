/*
    SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_CODE39BARCODE_H
#define PRISON_CODE39BARCODE_H

#include "abstractbarcode_p.h"

namespace Prison
{
/**
 * Code 39 Barcode generator
 */
class Code39Barcode : public Prison::AbstractBarcodePrivate
{
public:
    /**
     * creates a Code 39 generator
     */
    Code39Barcode();
    ~Code39Barcode() override;

protected:
    /**
     * This function generates the barcode
     * @return QImage containing a barcode, trying to approximate the requested sizes, or a null QImage if it can't be painted within requested size
     */
    QImage paintImage() override;
};
} // namespace

#endif // PRISON_CODE39BARCODE_H

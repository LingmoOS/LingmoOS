/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_DATAMATRIXBARCODE_H
#define PRISON_DATAMATRIXBARCODE_H

#include "abstractbarcode_p.h"
#include "prison_export.h"

namespace Prison
{
/**
 * This is a Datamatrix barcode generator that uses libdmtx
 * for the actual generation of barcodes.
 */
class DataMatrixBarcode : public Prison::AbstractBarcodePrivate
{
public:
    /**
     * creates a datamatrixbarcode generator
     */
    DataMatrixBarcode();
    ~DataMatrixBarcode() override;

protected:
    /**
     * This is the function doing the actual work in generating the barcode
     * @return QImage containing a DataMatrix, trying to approximate the requested sizes
     */
    QImage paintImage() override;
};
}

#endif // PRISON_DATAMATRIXBARCODE_H

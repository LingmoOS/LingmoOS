/*
    SPDX-FileCopyrightText: 2010-2014 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#include "datamatrixbarcode_p.h"
#include <dmtx.h>
using namespace Prison;

DataMatrixBarcode::DataMatrixBarcode()
    : AbstractBarcodePrivate(Barcode::TwoDimensions)
{
}
DataMatrixBarcode::~DataMatrixBarcode() = default;

QImage DataMatrixBarcode::paintImage()
{
    const auto data = m_data.toString();
    if (data.size() > 1200) {
        return QImage();
    }

    DmtxEncode *enc = dmtxEncodeCreate();
    dmtxEncodeSetProp(enc, DmtxPropPixelPacking, DmtxPack32bppRGBX);
    dmtxEncodeSetProp(enc, DmtxPropModuleSize, 1);
    dmtxEncodeSetProp(enc, DmtxPropMarginSize, 2);

    QByteArray trimmedData(data.trimmed().toUtf8());
    DmtxPassFail result = dmtxEncodeDataMatrix(enc, trimmedData.length(), reinterpret_cast<unsigned char *>(trimmedData.data()));
    if (result == DmtxFail) {
        dmtxEncodeDestroy(&enc);
        return QImage();
    }
    Q_ASSERT(enc->image->width == enc->image->height);

    QImage ret;

    if (m_foreground == Qt::black && m_background == Qt::white) {
        QImage tmp(enc->image->pxl, enc->image->width, enc->image->height, QImage::Format_ARGB32);
        // we need to copy, because QImage generated from a char pointer requires the
        // char pointer to be kept around forever, and manually deleted.
        ret = tmp.copy();
    } else {
        if (enc->image->width > 0) {
            int size = enc->image->width * enc->image->height * 4;
            uchar *img = new uchar[size];
            QByteArray background(4, '\0');
            background[3] = qAlpha(m_background.rgba());
            background[2] = qRed(m_background.rgba());
            background[1] = qGreen(m_background.rgba());
            background[0] = qBlue(m_background.rgba());
            QByteArray foreground(4, '\0');
            foreground[3] = qAlpha(m_foreground.rgba());
            foreground[2] = qRed(m_foreground.rgba());
            foreground[1] = qGreen(m_foreground.rgba());
            foreground[0] = qBlue(m_foreground.rgba());
            for (int i = 1; i < size; i += 4) {
                QByteArray color;
                if (enc->image->pxl[i] == 0x00) {
                    color = foreground;
                } else {
                    color = background;
                }
                for (int j = 0; j < 4; j++) {
                    img[i - 1 + j] = color[j];
                }
            }
            QImage tmp(img, enc->image->width, enc->image->height, QImage::Format_ARGB32);
            // we need to copy, because QImage generated from a char pointer requires the
            // char pointer to be kept around forever, and manually deleted.
            ret = tmp.copy();
            delete[] img;
        }
    }
    dmtxEncodeDestroy(&enc);
    return ret;
}

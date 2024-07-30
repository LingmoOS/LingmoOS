/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_ABSTRACTBARCODE_P_H
#define PRISON_ABSTRACTBARCODE_P_H

#include "barcode.h"
#include "prison.h"

#include <QImage>
#include <QVariant>

namespace Prison
{
class AbstractBarcodePrivate
{
public:
    explicit AbstractBarcodePrivate(Barcode::Dimensions dim);
    virtual ~AbstractBarcodePrivate();

    /**
     * Doing the actual painting of the image
     * @param size unused - will be removed in KF6
     * @return image with barcode, or null image
     */
    virtual QImage paintImage() = 0;

    /** @see Barcode::preferredSize */
    virtual QSizeF preferredSize(qreal devicePixelRatio) const;

    bool isEmpty() const;
    bool sizeTooSmall(const QSizeF &size) const;
    void recompute();

    QVariant m_data;
    QImage m_cache;
    QColor m_foreground = Qt::black;
    QColor m_background = Qt::white;
    Barcode::Dimensions m_dimension = Barcode::NoDimensions;
    Prison::BarcodeType m_format;
};

}

#endif

/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include <config-prison.h>

#include "barcode.h"

#include "abstractbarcode_p.h"
#include "aztecbarcode_p.h"
#include "code128barcode_p.h"
#include "code39barcode_p.h"
#include "code93barcode_p.h"
#include "datamatrixbarcode_p.h"
#include "pdf417barcode_p.h"
#include "qrcodebarcode_p.h"
#if HAVE_ZXING
#include "zxingonedbarcode_p.h"
#endif

#include <QColor>
#include <QPainter>
#include <QVariant>

using namespace Prison;

std::optional<Barcode> Barcode::create(Prison::BarcodeType format)
{
    std::unique_ptr<AbstractBarcodePrivate> d;
    switch (format) {
    case Prison::QRCode:
        d = std::make_unique<QRCodeBarcode>();
        break;
    case Prison::DataMatrix:
#if HAVE_DMTX
        d = std::make_unique<DataMatrixBarcode>();
#endif
        break;
    case Prison::Aztec:
        d = std::make_unique<AztecBarcode>();
        break;
    case Prison::Code39:
        d = std::make_unique<Code39Barcode>();
        break;
    case Prison::Code93:
        d = std::make_unique<Code93Barcode>();
        break;
    case Prison::Code128:
        d = std::make_unique<Code128Barcode>();
        break;
    case Prison::PDF417:
#if HAVE_ZXING
        d = std::make_unique<Pdf417Barcode>();
#endif
        break;
    case Prison::EAN13:
#if HAVE_ZXING
        d = std::make_unique<ZXingOneDBarcode<ZXing::BarcodeFormat::EAN13>>();
#endif
        break;
    }

    if (d) {
        d->m_format = format;
        return Barcode(std::move(d));
    }
    return std::nullopt;
}

Barcode::Barcode(std::unique_ptr<AbstractBarcodePrivate> &&dd)
    : d(std::move(dd))
{
}

Barcode::Barcode(Barcode &&) = default;
Barcode::~Barcode() = default;
Barcode &Barcode::operator=(Barcode &&) = default;

Prison::BarcodeType Barcode::format() const
{
    return d->m_format;
}

QString Barcode::data() const
{
    return d->m_data.userType() == QMetaType::QString ? d->m_data.toString() : QString();
}

QByteArray Barcode::byteArrayData() const
{
    return d->m_data.userType() == QMetaType::QByteArray ? d->m_data.toByteArray() : QByteArray();
}

QImage Barcode::toImage(const QSizeF &size)
{
    d->recompute();
    if (d->m_cache.isNull() || d->sizeTooSmall(size)) {
        return QImage();
    }

    // scale to the requested size, using only full integer factors to keep the code readable
    int scaleX = std::max<int>(1, size.width() / d->m_cache.width());
    int scaleY = std::max<int>(1, size.height() / d->m_cache.height());
    if (dimensions() == TwoDimensions) {
        scaleX = scaleY = std::min(scaleX, scaleY);
    }

    QImage out(d->m_cache.width() * scaleX, d->m_cache.height() * scaleY, d->m_cache.format());
    QPainter p(&out);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
    p.drawImage(out.rect(), d->m_cache, d->m_cache.rect());
    return out;
}

void Barcode::setData(const QString &data)
{
    if (d) {
        d->m_data = data;
        d->m_cache = QImage();
    }
}

void Barcode::setData(const QByteArray &data)
{
    d->m_data = data;
    d->m_cache = QImage();
}

QSizeF Barcode::minimumSize() const
{
    d->recompute();
    return d->m_cache.size();
}

QSizeF Barcode::preferredSize(qreal devicePixelRatio) const
{
    d->recompute();
    return d->preferredSize(devicePixelRatio);
}

QColor Barcode::backgroundColor() const
{
    return d->m_background;
}

QColor Barcode::foregroundColor() const
{
    return d->m_foreground;
}

void Barcode::setBackgroundColor(const QColor &backgroundcolor)
{
    if (backgroundcolor != backgroundColor()) {
        d->m_background = backgroundcolor;
        d->m_cache = QImage();
    }
}

void Barcode::setForegroundColor(const QColor &foregroundcolor)
{
    if (foregroundcolor != foregroundColor()) {
        d->m_foreground = foregroundcolor;
        d->m_cache = QImage();
    }
}

Barcode::Dimensions Barcode::dimensions() const
{
    return d->m_dimension;
}

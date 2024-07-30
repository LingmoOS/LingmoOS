/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "barcodequickitem.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>

using namespace Prison;

BarcodeQuickItem::BarcodeQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

BarcodeQuickItem::~BarcodeQuickItem() = default;

QVariant BarcodeQuickItem::content() const
{
    return m_content;
}

void BarcodeQuickItem::setContent(const QVariant &content)
{
    if (m_content == content) {
        return;
    }
    m_content = content;
    Q_EMIT contentChanged();
    updateBarcode();
}

QJSValue BarcodeQuickItem::barcodeType() const
{
    if (m_type) {
        return static_cast<BarcodeType>(m_type.value());
    }
    return QJSValue();
}

void BarcodeQuickItem::setBarcodeType(const QJSValue &type)
{
    if (!type.isNumber()) {
        if (m_type) {
            m_type.reset();
        } else {
            return;
        }

    } else {
        auto enumType = static_cast<Prison::BarcodeType>(type.toInt());
        if (enumType == m_type) {
            return;
        }
        m_type = enumType;
    }
    Q_EMIT barcodeTypeChanged();
    m_barcode.reset();
    updateBarcode();
}

QColor BarcodeQuickItem::foregroundColor() const
{
    return m_fgColor;
}

void BarcodeQuickItem::setForegroundColor(const QColor &color)
{
    if (m_fgColor == color) {
        return;
    }
    m_fgColor = color;
    Q_EMIT foregroundColorChanged();
    updateBarcode();
}

QColor BarcodeQuickItem::backgroundColor() const
{
    return m_bgColor;
}

void BarcodeQuickItem::setBackgroundColor(const QColor &color)
{
    if (m_bgColor == color) {
        return;
    }
    m_bgColor = color;
    Q_EMIT backgroundColorChanged();
    updateBarcode();
}

BarcodeQuickItem::Dimensions Prison::BarcodeQuickItem::dimensions() const
{
    if (m_barcode)
        return static_cast<BarcodeQuickItem::Dimensions>(m_barcode->dimensions());
    return BarcodeQuickItem::Dimensions::NoDimensions;
}

void BarcodeQuickItem::paint(QPainter *painter)
{
    if (!m_barcode) {
        return;
    }

    const auto w_max = std::max(minimumWidth(), width());
    const auto h_max = std::max(minimumHeight(), height());
    const auto img = m_barcode->toImage(QSizeF(w_max, h_max));
    const auto x = (w_max - img.width()) / 2;
    const auto y = (h_max - img.height()) / 2;
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawImage(QRectF(x, y, img.width(), img.height()), img, img.rect());
}

void BarcodeQuickItem::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    updateBarcode();
}

qreal BarcodeQuickItem::minimumHeight() const
{
    return m_barcode ? m_barcode->minimumSize().height() : 0;
}

qreal BarcodeQuickItem::minimumWidth() const
{
    return m_barcode ? m_barcode->minimumSize().width() : 0;
}

bool BarcodeQuickItem::isEmpty() const
{
    switch (m_content.userType()) {
    case QMetaType::QString:
        return m_content.toString().isEmpty();
    case QMetaType::QByteArray:
        return m_content.toByteArray().isEmpty();
    default:
        break;
    }
    return true;
}

void BarcodeQuickItem::updateBarcode()
{
    if (!isComponentComplete()) {
        return;
    }

    if (isEmpty() || !m_type) {
        m_barcode.reset();
        update();
        Q_EMIT dimensionsChanged();
        return;
    }
    if (!m_barcode) {
        m_barcode = Prison::Barcode::create(m_type.value());
    }
    if (!m_barcode) {
        return;
    }

        if (m_content.userType() == QMetaType::QString) {
            m_barcode->setData(m_content.toString());
        } else {
            m_barcode->setData(m_content.toByteArray());
        }
        m_barcode->setForegroundColor(m_fgColor);
        m_barcode->setBackgroundColor(m_bgColor);
        const auto size = m_barcode->preferredSize(QGuiApplication::primaryScreen()->devicePixelRatio());
        setImplicitSize(size.width(), size.height());

        update();
        Q_EMIT dimensionsChanged();
}

#include "moc_barcodequickitem.cpp"

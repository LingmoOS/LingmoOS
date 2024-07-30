/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_BARCODEQUICKITEM_H
#define PRISON_BARCODEQUICKITEM_H

#include <Prison/Barcode>

#include <QColor>
#include <QQuickPaintedItem>

#include <qqmlregistration.h>

namespace Prison
{

class BarcodeQuickItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(Barcode)
    Q_PROPERTY(QVariant content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(QJSValue barcodeType READ barcodeType WRITE setBarcodeType NOTIFY barcodeTypeChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(Dimensions dimensions READ dimensions NOTIFY dimensionsChanged)
    /**
     * @see Prison::Barcode::minimumSize()
     * @since 5.69
     */
    Q_PROPERTY(qreal minimumHeight READ minimumHeight NOTIFY implicitHeightChanged)
    /**
     * @see Prison::Barcode::minimumSize()
     * @since 5.69
     */
    Q_PROPERTY(qreal minimumWidth READ minimumWidth NOTIFY implicitWidthChanged)

public:
    enum BarcodeType {
        QRCode = Prison::QRCode,
        DataMatrix = Prison::DataMatrix,
        Aztec = Prison::Aztec,
        Code39 = Prison::Code39,
        Code93 = Prison::Code93,
        Code128 = Prison::Code128,
        PDF417 = Prison::PDF417,
        EAN13 = Prison::EAN13,
    };
    Q_ENUM(BarcodeType)
    explicit BarcodeQuickItem(QQuickItem *parent = nullptr);
    ~BarcodeQuickItem() override;

    QVariant content() const;
    void setContent(const QVariant &data);

    QJSValue barcodeType() const;
    void setBarcodeType(const QJSValue &type);

    QColor foregroundColor() const;
    void setForegroundColor(const QColor &color);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

    enum Dimensions {
        NoDimensions,
        OneDimension,
        TwoDimensions,
    };
    Q_ENUM(Dimensions)
    Dimensions dimensions() const;

    void paint(QPainter *painter) override;
    void componentComplete() override;

    qreal minimumHeight() const;
    qreal minimumWidth() const;

Q_SIGNALS:
    void contentChanged();
    void barcodeTypeChanged();
    void foregroundColorChanged();
    void backgroundColorChanged();
    void dimensionsChanged();

private:
    bool isEmpty() const;
    void updateBarcode();

    QVariant m_content;
    std::optional<Barcode> m_barcode;
    QColor m_fgColor = Qt::black;
    QColor m_bgColor = Qt::white;
    std::optional<Prison::BarcodeType> m_type;
};

}

#endif // PRISON_BARCODEQUICKITEM_H

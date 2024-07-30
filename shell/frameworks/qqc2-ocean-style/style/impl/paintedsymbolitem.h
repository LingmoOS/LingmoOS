/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PAINTEDSYMBOLITEM_H
#define PAINTEDSYMBOLITEM_H

#include <QQuickPaintedItem>
#include <climits>

class PaintedSymbolItemPrivate;

/**
 * This class is mainly intended for drawing symbols that are difficult to make
 * with QQuickRectangle or that don't look right with QQuickRectangle.
 *
 * This should be used instead of the Qt Quick Shapes API because
 * Qt Quick Shapes can't use good antialiasing. You can use MSAA (expensive) on
 * Qt Quick Shapes via the `layer.samples` property, but it's very prone to
 * distortion when you change the dimentions of the symbol and distortion issues
 * can be difficult to reliably reproduce.
 */
class PaintedSymbolItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    // Should be used when setting the size of this element
    Q_PROPERTY(qreal penWidth READ penWidth NOTIFY penWidthChanged)
    Q_PROPERTY(SymbolType symbolType READ symbolType WRITE setSymbolType NOTIFY symbolTypeChanged)
    QML_NAMED_ELEMENT(PaintedSymbol)

public:
    enum SymbolType {
        Checkmark,
        LeftArrow,
        RightArrow,
        UpArrow,
        DownArrow,
        NSymbolTypes, // Only used to access the total number of symbol types
        None = USHRT_MAX,
    };
    Q_ENUM(SymbolType)

    explicit PaintedSymbolItem(QQuickItem *parent = nullptr);
    ~PaintedSymbolItem();

    void paint(QPainter *painter) override;

    QColor color() const;
    void setColor(const QColor &color);

    qreal penWidth() const;
    void setPenWidth(const qreal penWidth);

    SymbolType symbolType() const;
    void setSymbolType(const SymbolType symbolType);

Q_SIGNALS:
    void colorChanged();
    void penWidthChanged();
    void symbolTypeChanged();

private:
    const QScopedPointer<PaintedSymbolItemPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PaintedSymbolItem)
    Q_DISABLE_COPY(PaintedSymbolItem)
};

#endif

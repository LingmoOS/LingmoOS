/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
 */

#ifndef ICONLABELLAYOUT_H
#define ICONLABELLAYOUT_H

#include "qquickicon_p.h"
#include <QQuickItem>
#include <memory>

class IconLabelLayoutPrivate;

/**
 * An item that is meant to show, hide and position an icon item and a label item based on an AbstractButton's display property.
 *
 * You can also set the alignment of the icon and label as a group.
 *
 * For example, this aligns the icon and label to the left side and vertical center of this item.
 *
 * ```qml
 * alignment: Qt.AlignLeft | Qt.AlignVCenter
 * ```
 *
 * The result will look like this if there is enough availableWidth and the display type is TextUnderIcon:
 *
 * ```
 * +-----------------+
 * |                 |
 * |   ▶             |
 * | label           |
 * |                 |
 * +-----------------+
 * ```
 *
 * The default alignment is Qt.AlignCenter and the default display type is TextBesideIcon, which looks like this:
 *
 * ```
 * +-----------------+
 * |                 |
 * |     ▶ label     |
 * |                 |
 * +-----------------+
 * ```
 *
 * You cannot separate the icon and label from each other.
 * They will always move together in the orientation specified by the display property.
 *
 * @note: This class was adapted from QtQuick.Control.impl IconLabel
 */
class IconLabelLayout : public QQuickItem
{
    Q_OBJECT
    /* Using components because I don't want anyone to mess with the icon or label items externally.
     * Unfortunately, it's not possible to just create items directly.
     * For the icon, I could reimplement LingmoUI Icon, but that would be more work and code to manage.
     * There's no point in using this layout if you're not going to define both, so both are required.
     * There's also no point in allowing the components to be read externally.
     */
    Q_PROPERTY(QQmlComponent *iconComponent READ iconComponent WRITE setIconComponent FINAL REQUIRED)
    Q_PROPERTY(QQmlComponent *labelComponent READ labelComponent WRITE setLabelComponent FINAL REQUIRED)

    Q_PROPERTY(bool hasIcon READ hasIcon NOTIFY hasIconChanged FINAL)
    Q_PROPERTY(bool hasLabel READ hasLabel NOTIFY hasLabelChanged FINAL)

    // qqc2-ocean-style has its own copy/pasted implementation of QQuickIcon
    Q_PROPERTY(Ocean::QQuickIcon icon READ icon WRITE setIcon NOTIFY iconChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    // convenient for aligning things with the icon or label
    Q_PROPERTY(QRectF iconRect READ iconRect NOTIFY iconRectChanged FINAL)
    Q_PROPERTY(QRectF labelRect READ labelRect NOTIFY labelRectChanged FINAL)

    // The width or height of IconLabelLayout, minus left and right padding or top and bottom padding
    Q_PROPERTY(qreal availableWidth READ availableWidth NOTIFY availableWidthChanged FINAL)
    Q_PROPERTY(qreal availableHeight READ availableHeight NOTIFY availableHeightChanged FINAL)

    // The amount of space between the icon and the label.
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing NOTIFY spacingChanged FINAL)

    // The minimum amount of space around the icon and label.
    Q_PROPERTY(qreal leftPadding READ leftPadding WRITE setLeftPadding NOTIFY leftPaddingChanged FINAL)
    Q_PROPERTY(qreal rightPadding READ rightPadding WRITE setRightPadding NOTIFY rightPaddingChanged FINAL)
    Q_PROPERTY(qreal topPadding READ topPadding WRITE setTopPadding NOTIFY topPaddingChanged FINAL)
    Q_PROPERTY(qreal bottomPadding READ bottomPadding WRITE setBottomPadding NOTIFY bottomPaddingChanged FINAL)

    // Whether or not a right to left layout should be used
    Q_PROPERTY(bool mirrored READ mirrored WRITE setMirrored NOTIFY mirroredChanged FINAL)

    // The position of the icon and label as a group within IconLabelLayout.
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged FINAL)

    // The visibility and position of icon and label relative to each other.
    Q_PROPERTY(Display display READ display WRITE setDisplay NOTIFY displayChanged FINAL)

    // Convenience functions to check what display type is being used
    Q_PROPERTY(bool iconOnly READ iconOnly NOTIFY iconOnlyChanged FINAL)
    Q_PROPERTY(bool textOnly READ textOnly NOTIFY textOnlyChanged FINAL)
    Q_PROPERTY(bool textBesideIcon READ textBesideIcon NOTIFY textBesideIconChanged FINAL)
    Q_PROPERTY(bool textUnderIcon READ textUnderIcon NOTIFY textUnderIconChanged FINAL)

    QML_NAMED_ELEMENT(IconLabelLayout)

public:
    enum Display {
        IconOnly,
        TextOnly,
        TextBesideIcon,
        TextUnderIcon,
    };
    Q_ENUM(Display)

    explicit IconLabelLayout(QQuickItem *parent = nullptr);
    ~IconLabelLayout();

    QQmlComponent *iconComponent() const;
    void setIconComponent(QQmlComponent *iconComponent);

    QQmlComponent *labelComponent() const;
    void setLabelComponent(QQmlComponent *labelComponent);

    bool hasIcon() const;
    void setHasIcon();
    bool hasLabel() const;
    void setHasLabel();

    Ocean::QQuickIcon icon() const;
    void setIcon(const Ocean::QQuickIcon &icon);

    QString text() const;
    void setText(const QString &text);

    QFont font() const;
    void setFont(const QFont &font);

    QColor color() const;
    void setColor(const QColor &color);

    QRectF iconRect() const;
    void setIconRect(const QRectF &rect);
    QRectF labelRect() const;
    void setLabelRect(const QRectF &rect);

    qreal availableWidth() const;
    void setAvailableWidth();
    qreal availableHeight() const;
    void setAvailableHeight();

    qreal spacing() const;
    void setSpacing(qreal spacing);
    void resetSpacing();

    qreal leftPadding() const;
    void setLeftPadding(qreal leftPadding);
    void resetLeftPadding();

    qreal rightPadding() const;
    void setRightPadding(qreal rightPadding);
    void resetRightPadding();

    qreal topPadding() const;
    void setTopPadding(qreal topPadding);
    void resetTopPadding();

    qreal bottomPadding() const;
    void setBottomPadding(qreal bottomPadding);
    void resetBottomPadding();

    bool mirrored() const;
    void setMirrored(bool mirrored);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    Display display() const;
    void setDisplay(Display display);

    bool iconOnly() const;
    bool textOnly() const;
    bool textBesideIcon() const;
    bool textUnderIcon() const;

Q_SIGNALS:
    void iconComponentChanged();
    void labelComponentChanged();
    void hasIconChanged();
    void hasLabelChanged();
    void iconChanged();
    void textChanged(const QString &text);
    void fontChanged(const QFont &font);
    void colorChanged();
    void iconRectChanged();
    void labelRectChanged();

    void availableWidthChanged();
    void availableHeightChanged();

    void spacingChanged();
    void topPaddingChanged();
    void leftPaddingChanged();
    void rightPaddingChanged();
    void bottomPaddingChanged();

    void mirroredChanged();
    void alignmentChanged();
    void displayChanged();
    void iconOnlyChanged();
    void textOnlyChanged();
    void textBesideIconChanged();
    void textUnderIconChanged();

public:
    Q_SLOT void relayout();

protected:
    void componentComplete() override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    const QScopedPointer<IconLabelLayoutPrivate> d_ptr;
    Q_DECLARE_PRIVATE(IconLabelLayout)
    Q_DISABLE_COPY(IconLabelLayout)
};

QML_DECLARE_TYPE(IconLabelLayout)

#endif

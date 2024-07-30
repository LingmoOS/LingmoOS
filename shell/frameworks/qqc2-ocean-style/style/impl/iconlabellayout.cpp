/* SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
 */

#include "iconlabellayout.h"
#include "iconlabellayout_p.h"

#include <QGuiApplication>

#include <cmath>
#include <unordered_map>

bool IconLabelLayoutPrivate::createIconItem()
{
    Q_Q(IconLabelLayout);
    if (iconItem)
        return false;

    iconItem = qobject_cast<QQuickItem *>(iconComponent->create());
    iconItem->setParentItem(q);
    iconItem->setObjectName(QStringLiteral("iconItem"));
    iconItem->setProperty("source", icon.nameOrSource());
    iconItem->setProperty("implicitWidth", icon.width());
    iconItem->setProperty("implicitHeight", icon.height());
    iconItem->setProperty("color", icon.color());
    iconItem->setProperty("cache", icon.cache());
    return true;
}

bool IconLabelLayoutPrivate::destroyIconItem()
{
    if (!iconItem)
        return false;

    iconItem->deleteLater();
    iconItem = nullptr;
    return true;
}

bool IconLabelLayoutPrivate::updateIconItem()
{
    Q_Q(IconLabelLayout);
    if (!q->hasIcon())
        return destroyIconItem();
    return createIconItem();
}

void IconLabelLayoutPrivate::syncIconItem()
{
    if (!iconItem || icon.isEmpty())
        return;

    iconItem->setProperty("source", icon.nameOrSource());
    iconItem->setProperty("implicitWidth", icon.width());
    iconItem->setProperty("implicitHeight", icon.height());
    iconItem->setProperty("color", icon.color());
    iconItem->setProperty("cache", icon.cache());
}

void IconLabelLayoutPrivate::updateOrSyncIconItem()
{
    Q_Q(IconLabelLayout);
    if (!updateIconItem()) {
        syncIconItem();
    }
    q->relayout();
}

bool IconLabelLayoutPrivate::createLabelItem()
{
    Q_Q(IconLabelLayout);
    if (labelItem)
        return false;

    labelItem = qobject_cast<QQuickItem *>(labelComponent->create());
    labelItem->setParentItem(q);
    labelItem->setObjectName(QStringLiteral("labelItem"));
    labelItem->setProperty("text", text);
    labelItem->setProperty("font", font);
    labelItem->setProperty("color", color);
    const int halign = alignment & Qt::AlignHorizontal_Mask;
    labelItem->setProperty("horizontalAlignment", halign);
    const int valign = alignment & Qt::AlignVertical_Mask;
    labelItem->setProperty("verticalAlignment", valign);
    return true;
}

bool IconLabelLayoutPrivate::destroyLabelItem()
{
    if (!labelItem)
        return false;

    labelItem->deleteLater();
    labelItem = nullptr;
    return true;
}

bool IconLabelLayoutPrivate::updateLabelItem()
{
    Q_Q(IconLabelLayout);
    if (!q->hasLabel())
        return destroyLabelItem();
    return createLabelItem();
}

void IconLabelLayoutPrivate::syncLabelItem()
{
    if (!labelItem)
        return;

    labelItem->setProperty("text", text);
}

void IconLabelLayoutPrivate::updateOrSyncLabelItem()
{
    Q_Q(IconLabelLayout);
    if (!updateLabelItem()) {
        syncLabelItem();
    }
    q->relayout();
}

void IconLabelLayoutPrivate::updateImplicitSize()
{
    Q_Q(IconLabelLayout);
    bool showIcon = iconItem && q->hasIcon();
    bool showLabel = labelItem && q->hasLabel();

    const qreal iconImplicitWidth = showIcon ? iconItem->implicitWidth() : 0;
    const qreal iconImplicitHeight = showIcon ? iconItem->implicitHeight() : 0;
    // Always ceil text size to prevent pixel misalignment. If you use round or floor, you may cause text elision.
    const qreal labelImplicitWidth = showLabel ? std::ceil(labelItem->implicitWidth()) : 0;
    const qreal labelImplicitHeight = showLabel ? std::ceil(labelItem->implicitHeight()) : 0;
    const qreal effectiveSpacing = showLabel && showIcon && iconItem->implicitWidth() > 0 ? spacing : 0;
    contentWidth = (display == IconLabelLayout::TextBesideIcon //
                        ? iconImplicitWidth + labelImplicitWidth + effectiveSpacing
                        : qMax(iconImplicitWidth, labelImplicitWidth));
    contentHeight = (display == IconLabelLayout::TextUnderIcon //
                         ? iconImplicitHeight + labelImplicitHeight + effectiveSpacing
                         : qMax(iconImplicitHeight, labelImplicitHeight));
    q->setImplicitSize(contentWidth + leftPadding + rightPadding, contentHeight + topPadding + bottomPadding);
    q->setAvailableWidth();
    q->setAvailableHeight();
}

static QRectF alignedRect(bool mirrored, Qt::Alignment alignment, const QSizeF &size, const QRectF &rectangle)
{
    Qt::Alignment halign = alignment & Qt::AlignHorizontal_Mask;
    if (mirrored && (halign & Qt::AlignRight) == Qt::AlignRight) {
        halign = Qt::AlignLeft;
    } else if (mirrored && (halign & Qt::AlignLeft) == Qt::AlignLeft) {
        halign = Qt::AlignRight;
    }
    qreal x = rectangle.x();
    qreal y = rectangle.y();
    const qreal w = size.width();
    const qreal h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += rectangle.height() / 2 - h / 2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rectangle.height() - h;
    if ((halign & Qt::AlignRight) == Qt::AlignRight)
        x += rectangle.width() - w;
    else if ((halign & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += rectangle.width() / 2 - w / 2;
    return QRectF(x, y, w, h);
}

void IconLabelLayoutPrivate::layout()
{
    Q_Q(IconLabelLayout);
    if (!q->isComponentComplete())
        return;

    switch (display) {
    case IconLabelLayout::IconOnly:
        if (iconItem) {
            // Icons should always be pixel aligned, so convert to QRect
            QSizeF size(qMin(iconItem->implicitWidth(), q->availableWidth()), qMin(iconItem->implicitHeight(), q->availableHeight()));
            QRectF rect(mirrored ? rightPadding : leftPadding, topPadding, q->availableWidth(), q->availableHeight());
            q->setIconRect(alignedRect(mirrored, alignment, size, rect));
            iconItem->setSize(iconRect.size());
            iconItem->setPosition(iconRect.topLeft()); // Not animating icon positions because it tends to look wrong
        }
        break;
    case IconLabelLayout::TextOnly:
        if (labelItem) {
            QSizeF size(qMin(labelItem->implicitWidth(), q->availableWidth()), qMin(labelItem->implicitHeight(), q->availableHeight()));
            QRectF rect(mirrored ? rightPadding : leftPadding, topPadding, q->availableWidth(), q->availableHeight());
            q->setLabelRect(alignedRect(mirrored, alignment, size, rect));
            labelItem->setSize(labelRect.size());
            labelItem->setPosition(labelRect.topLeft()); // Not animating when text only because the text tends to clip outside
        }
        break;

    case IconLabelLayout::TextUnderIcon: {
        // Work out the sizes first, as the positions depend on them.
        QSizeF iconSize;
        QSizeF textSize;
        if (iconItem) {
            iconSize.setWidth(qMin(iconItem->implicitWidth(), q->availableWidth()));
            iconSize.setHeight(qMin(iconItem->implicitHeight(), q->availableHeight()));
        }
        qreal effectiveSpacing = 0;
        if (labelItem) {
            if (!iconSize.isEmpty())
                effectiveSpacing = spacing;
            textSize.setWidth(qMin(labelItem->implicitWidth(), q->availableWidth()));
            textSize.setHeight(qMin(labelItem->implicitHeight(), q->availableHeight() - iconSize.height() - effectiveSpacing));
        }

        QSizeF size(qMax(iconSize.width(), textSize.width()), iconSize.height() + effectiveSpacing + textSize.height());
        QRectF rect(mirrored ? rightPadding : leftPadding, topPadding, q->availableWidth(), q->availableHeight());
        QRectF combinedRect = alignedRect(mirrored, alignment, size, rect);
        q->setIconRect(alignedRect(mirrored, Qt::AlignHCenter | Qt::AlignTop, iconSize, combinedRect));
        q->setLabelRect(alignedRect(mirrored, Qt::AlignHCenter | Qt::AlignBottom, textSize, combinedRect));
        if (iconItem) {
            iconItem->setSize(iconRect.size());
            iconItem->setPosition(iconRect.topLeft());
        }
        if (labelItem) {
            labelItem->setSize(labelRect.size());
            labelItem->setPosition(labelRect.topLeft());
            // NOTE: experimental animations when changing display types
            // labelItem->setOpacity(0); // Reset opacity before OpacityAnimator is activated
            // labelItem->setY(iconRect.y() + iconRect.height());
            // labelItem->setProperty("opacity", 1); // Activate OpacityAnimator
            // labelItem->setX(labelRect.x()); // Not animating X so that the text will only slide vertically
            // labelItem->setProperty("y", labelRect.y());
        }
        break;
    }

    case IconLabelLayout::TextBesideIcon:
    default:
        // Work out the sizes first, as the positions depend on them.
        QSizeF iconSize(0, 0);
        QSizeF textSize(0, 0);
        if (iconItem) {
            iconSize.setWidth(qMin(iconItem->implicitWidth(), q->availableWidth()));
            iconSize.setHeight(qMin(iconItem->implicitHeight(), q->availableHeight()));
        }
        qreal effectiveSpacing = 0;
        if (labelItem) {
            if (!iconSize.isEmpty())
                effectiveSpacing = spacing;
            textSize.setWidth(qMin(labelItem->implicitWidth(), q->availableWidth() - iconSize.width() - effectiveSpacing));
            textSize.setHeight(qMin(labelItem->implicitHeight(), q->availableHeight()));
        }

        QSizeF size(iconSize.width() + effectiveSpacing + textSize.width(), qMax(iconSize.height(), textSize.height()));
        QRectF rect(mirrored ? rightPadding : leftPadding, topPadding, q->availableWidth(), q->availableHeight());
        const QRectF combinedRect = alignedRect(mirrored, alignment, size, rect);
        q->setIconRect(alignedRect(mirrored, Qt::AlignLeft | Qt::AlignVCenter, iconSize, combinedRect));
        q->setLabelRect(alignedRect(mirrored, Qt::AlignRight | Qt::AlignVCenter, textSize, combinedRect));
        if (iconItem) {
            iconItem->setSize(iconRect.size());
            iconItem->setPosition(iconRect.topLeft());
        }
        if (labelItem) {
            labelItem->setSize(labelRect.size());
            labelItem->setPosition(labelRect.topLeft());
            // NOTE: experimental animations when changing display types
            // labelItem->setOpacity(0); // Reset opacity before OpacityAnimator is activated
            // labelItem->setX(iconRect.x() + (mirrored ? -labelRect.width() : iconRect.width()));
            // labelItem->setProperty("opacity", 1); // Activate OpacityAnimator
            // labelItem->setProperty("x", labelRect.x());
            // labelItem->setY(labelRect.y()); // Not animating Y so that the text will only slide horizontally
        }
        break;
    }

    q->setBaselineOffset(labelItem ? labelItem->y() + labelItem->baselineOffset() : 0);
    if (!firstLayoutCompleted) {
        firstLayoutCompleted = true;
        if (iconItem) {
            iconItem->setProperty("firstLayoutCompleted", true);
        }
        if (labelItem) {
            labelItem->setProperty("firstLayoutCompleted", true);
        }
    }
    // qDebug() << q << "d->layout()" << layoutCount;
    // layoutCount += 1;
}

IconLabelLayout::IconLabelLayout(QQuickItem *parent)
    : QQuickItem(parent)
    , d_ptr(new IconLabelLayoutPrivate(this))
{
}

IconLabelLayout::~IconLabelLayout()
{
}

QQmlComponent *IconLabelLayout::iconComponent() const
{
    Q_D(const IconLabelLayout);
    return d->iconComponent;
}

void IconLabelLayout::setIconComponent(QQmlComponent *iconComponent)
{
    Q_D(IconLabelLayout);
    if (iconComponent == d->iconComponent) {
        return;
    }

    d->iconComponent = iconComponent;
    d->updateOrSyncIconItem();
    Q_EMIT iconComponentChanged();
}

QQmlComponent *IconLabelLayout::labelComponent() const
{
    Q_D(const IconLabelLayout);
    return d->labelComponent;
}

void IconLabelLayout::setLabelComponent(QQmlComponent *labelComponent)
{
    Q_D(IconLabelLayout);
    if (labelComponent == d->labelComponent) {
        return;
    }

    d->labelComponent = labelComponent;
    d->updateOrSyncLabelItem();
    Q_EMIT labelComponentChanged();
}

bool IconLabelLayout::hasIcon() const
{
    Q_D(const IconLabelLayout);
    return d->hasIcon;
}

void IconLabelLayout::setHasIcon()
{
    Q_D(IconLabelLayout);
    if (d->hasIcon == !textOnly() && !d->icon.isEmpty()) {
        return;
    }

    d->hasIcon = !textOnly() && !d->icon.isEmpty();
    Q_EMIT hasIconChanged();
}

bool IconLabelLayout::hasLabel() const
{
    Q_D(const IconLabelLayout);
    return d->hasLabel;
}

void IconLabelLayout::setHasLabel()
{
    Q_D(IconLabelLayout);
    if (d->hasLabel == !iconOnly() && !d->text.isEmpty()) {
        return;
    }

    d->hasLabel = !iconOnly() && !d->text.isEmpty();
    Q_EMIT hasLabelChanged();
}

Ocean::QQuickIcon IconLabelLayout::icon() const
{
    Q_D(const IconLabelLayout);
    return d->icon;
}

void IconLabelLayout::setIcon(const Ocean::QQuickIcon &icon)
{
    Q_D(IconLabelLayout);
    if (icon == d->icon) {
        return;
    }

    d->icon = icon;
    setHasIcon();
    d->updateOrSyncIconItem();

    Q_EMIT iconChanged();
}

QString IconLabelLayout::text() const
{
    Q_D(const IconLabelLayout);
    return d->text;
}

void IconLabelLayout::setText(const QString &text)
{
    Q_D(IconLabelLayout);
    if (text == d->text) {
        return;
    }

    d->text = text;
    setHasLabel();
    d->updateOrSyncLabelItem();
    Q_EMIT textChanged(text);
}

QFont IconLabelLayout::font() const
{
    Q_D(const IconLabelLayout);
    return d->font;
}

void IconLabelLayout::setFont(const QFont &font)
{
    Q_D(IconLabelLayout);
    if (font == d->font) {
        return;
    }

    d->font = font;
    if (d->labelItem) {
        d->labelItem->setProperty("font", font);
    }
    d->updateOrSyncLabelItem();
    Q_EMIT fontChanged(font);
}

QColor IconLabelLayout::color() const
{
    Q_D(const IconLabelLayout);
    return d->color;
}

void IconLabelLayout::setColor(const QColor &color)
{
    Q_D(IconLabelLayout);
    if (color == d->color) {
        return;
    }

    d->color = color;
    if (d->labelItem) {
        d->labelItem->setProperty("color", color);
    }

    Q_EMIT colorChanged();
}

QRectF IconLabelLayout::iconRect() const
{
    Q_D(const IconLabelLayout);
    return d->iconRect;
}

void IconLabelLayout::setIconRect(const QRectF &rect)
{
    Q_D(IconLabelLayout);
    // Icons should always be pixel aligned
    QRectF alignedRect = rect.toAlignedRect();
    if (d->iconRect == alignedRect) {
        return;
    }

    d->iconRect = alignedRect;
    Q_EMIT iconRectChanged();
}

QRectF IconLabelLayout::labelRect() const
{
    Q_D(const IconLabelLayout);
    return d->labelRect;
}

void IconLabelLayout::setLabelRect(const QRectF &rect)
{
    Q_D(IconLabelLayout);
    if (d->labelRect == rect) {
        return;
    }

    d->labelRect = rect;
    Q_EMIT labelRectChanged();
}

qreal IconLabelLayout::availableWidth() const
{
    Q_D(const IconLabelLayout);
    return d->availableWidth;
}

void IconLabelLayout::setAvailableWidth()
{
    Q_D(IconLabelLayout);
    qreal newAvailableWidth = std::max(0.0, width() - leftPadding() - rightPadding());
    if (d->availableWidth == newAvailableWidth) {
        return;
    }

    d->availableWidth = newAvailableWidth;
    Q_EMIT availableWidthChanged();
}

qreal IconLabelLayout::availableHeight() const
{
    Q_D(const IconLabelLayout);
    return d->availableHeight;
}

void IconLabelLayout::setAvailableHeight()
{
    Q_D(IconLabelLayout);
    qreal newAvailableHeight = std::max(0.0, height() - topPadding() - bottomPadding());
    if (d->availableHeight == newAvailableHeight) {
        return;
    }

    d->availableHeight = newAvailableHeight;
    Q_EMIT availableHeightChanged();
}

qreal IconLabelLayout::spacing() const
{
    Q_D(const IconLabelLayout);
    return d->spacing;
}

void IconLabelLayout::setSpacing(qreal spacing)
{
    Q_D(IconLabelLayout);
    if (spacing == d->spacing) {
        return;
    }

    d->spacing = spacing;
    Q_EMIT spacingChanged();
    if (d->iconItem && d->labelItem) {
        relayout();
    }
}

qreal IconLabelLayout::leftPadding() const
{
    Q_D(const IconLabelLayout);
    return d->leftPadding;
}

void IconLabelLayout::setLeftPadding(qreal leftPadding)
{
    Q_D(IconLabelLayout);
    if (leftPadding == d->leftPadding) {
        return;
    }

    d->leftPadding = leftPadding;
    Q_EMIT leftPaddingChanged();
    relayout();
}

qreal IconLabelLayout::rightPadding() const
{
    Q_D(const IconLabelLayout);
    return d->rightPadding;
}

void IconLabelLayout::setRightPadding(qreal rightPadding)
{
    Q_D(IconLabelLayout);
    if (rightPadding == d->rightPadding) {
        return;
    }

    d->rightPadding = rightPadding;
    Q_EMIT rightPaddingChanged();
    relayout();
}

qreal IconLabelLayout::topPadding() const
{
    Q_D(const IconLabelLayout);
    return d->topPadding;
}

void IconLabelLayout::setTopPadding(qreal topPadding)
{
    Q_D(IconLabelLayout);
    if (topPadding == d->topPadding) {
        return;
    }

    d->topPadding = topPadding;
    Q_EMIT topPaddingChanged();
    relayout();
}

qreal IconLabelLayout::bottomPadding() const
{
    Q_D(const IconLabelLayout);
    return d->bottomPadding;
}

void IconLabelLayout::setBottomPadding(qreal bottomPadding)
{
    Q_D(IconLabelLayout);
    if (bottomPadding == d->bottomPadding) {
        return;
    }

    d->bottomPadding = bottomPadding;
    Q_EMIT bottomPaddingChanged();
    relayout();
}

bool IconLabelLayout::mirrored() const
{
    Q_D(const IconLabelLayout);
    return d->mirrored;
}

void IconLabelLayout::setMirrored(bool mirrored)
{
    Q_D(IconLabelLayout);
    if (mirrored == d->mirrored) {
        return;
    }

    d->mirrored = mirrored;
    Q_EMIT mirroredChanged();
    if (isComponentComplete()) {
        d->layout();
    }
}

Qt::Alignment IconLabelLayout::alignment() const
{
    Q_D(const IconLabelLayout);
    return d->alignment;
}

void IconLabelLayout::setAlignment(Qt::Alignment alignment)
{
    Q_D(IconLabelLayout);
    const int valign = alignment & Qt::AlignVertical_Mask;
    const int halign = alignment & Qt::AlignHorizontal_Mask;
    const uint align = (valign ? valign : Qt::AlignVCenter) | (halign ? halign : Qt::AlignHCenter);
    if (d->alignment == align) {
        return;
    }

    d->alignment = static_cast<Qt::Alignment>(align);
    if (d->labelItem) {
        d->labelItem->setProperty("horizontalAlignment", halign);
        d->labelItem->setProperty("verticalAlignment", valign);
    }
    Q_EMIT alignmentChanged();
    if (isComponentComplete()) {
        d->layout();
    }
}

IconLabelLayout::Display IconLabelLayout::display() const
{
    Q_D(const IconLabelLayout);
    return d->display;
}

void IconLabelLayout::setDisplay(IconLabelLayout::Display display)
{
    Q_D(IconLabelLayout);
    Display oldDisplay = d->display;
    if (display == oldDisplay) {
        return;
    }

    d->display = display;
    Q_EMIT displayChanged();

    if (oldDisplay == Display::IconOnly || iconOnly()) {
        Q_EMIT iconOnlyChanged();
    } else if (oldDisplay == Display::TextOnly || textOnly()) {
        Q_EMIT textOnlyChanged();
    } else if (oldDisplay == Display::TextBesideIcon || textBesideIcon()) {
        Q_EMIT textBesideIconChanged();
    } else if (oldDisplay == Display::TextUnderIcon || textUnderIcon()) {
        Q_EMIT textUnderIconChanged();
    }

    setHasIcon();
    setHasLabel();

    d->updateIconItem();
    d->updateLabelItem();
    relayout();
}

bool IconLabelLayout::iconOnly() const
{
    Q_D(const IconLabelLayout);
    return d->display == Display::IconOnly;
}

bool IconLabelLayout::textOnly() const
{
    Q_D(const IconLabelLayout);
    return d->display == Display::TextOnly;
}

bool IconLabelLayout::textBesideIcon() const
{
    Q_D(const IconLabelLayout);
    return d->display == Display::TextBesideIcon;
}

bool IconLabelLayout::textUnderIcon() const
{
    Q_D(const IconLabelLayout);
    return d->display == Display::TextUnderIcon;
}

void IconLabelLayout::relayout()
{
    Q_D(IconLabelLayout);
    if (isComponentComplete()) {
        d->updateImplicitSize();
        d->layout();
    }
}

void IconLabelLayout::componentComplete()
{
    QQuickItem::componentComplete();
    relayout();
}

void IconLabelLayout::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry != oldGeometry) {
        setAvailableWidth();
        setAvailableHeight();
        relayout();
    }
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

#include "moc_iconlabellayout.cpp"

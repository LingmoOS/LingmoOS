// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickdciiconimage_p.h"
#include "dquickiconlabel_p.h"
#include "dquickiconlabel_p_p.h"
#include "dquickdciiconimage_p_p.h"

#include <DGuiApplicationHelper>
#include <DPlatformTheme>
#include <DIconTheme>

#include <private/qguiapplication_p.h>

DQUICK_BEGIN_NAMESPACE

static void beginClass(QQuickItem *item)
{
    if (QQmlParserStatus *parserStatus = qobject_cast<QQmlParserStatus *>(item))
        parserStatus->classBegin();
}

static void completeComponent(QQuickItem *item)
{
    if (QQmlParserStatus *parserStatus = qobject_cast<QQmlParserStatus *>(item))
        parserStatus->componentComplete();
}

bool DQuickIconLabelPrivate::hasIcon() const
{
    return display != DQuickIconLabel::TextOnly
            && (!icon.isEmpty());
}

bool DQuickIconLabelPrivate::hasText() const
{
    return display != DQuickIconLabel::IconOnly && !text.isEmpty();
}

void DQuickIconLabelPrivate::createIconImage()
{
    Q_Q(DQuickIconLabel);
    image = new DQuickDciIconImage(q);
    QQmlEngine::setContextForObject(image, QQmlEngine::contextForObject(q));
    watchChanges(image);
    beginClass(image);
    image->setObjectName(QStringLiteral("DciIconImage"));
    image->setName(icon.name());
    image->setTheme(icon.theme());
    image->setPalette(icon.palette());
    image->setSourceSize(iconSize());
    image->setMode(icon.mode());
    image->setFallbackToQIcon(icon.fallbackToQIcon());
    image->imageItem()->setFallbackSource(icon.source());
}

bool DQuickIconLabelPrivate::ensureImage()
{
    if (image)
        return false;

    createIconImage();
    if (componentComplete)
        completeComponent(image);
    return true;
}

bool DQuickIconLabelPrivate::destroyImage()
{
    if (!image)
        return false;

    unwatchChanges(image);
    delete image;
    image = nullptr;
    return true;
}

bool DQuickIconLabelPrivate::updateImage()
{
    if (!hasIcon())
        return destroyImage();
    return ensureImage();
}

void DQuickIconLabelPrivate::syncImage()
{
    if (!image || !hasIcon())
        return;

    image->setName(icon.name());
    image->setMode(icon.mode());
    image->setSourceSize(iconSize());
    image->setPalette(icon.palette());
    image->setTheme(icon.theme());
    image->setFallbackToQIcon(icon.fallbackToQIcon());
    image->imageItem()->setFallbackSource(icon.source());

    const int valign = static_cast<int>(alignment & Qt::AlignVertical_Mask);
    image->imageItem()->setVerticalAlignment(static_cast<QQuickImage::VAlignment>(valign));
    const int halign = static_cast<int>(alignment & Qt::AlignHorizontal_Mask);
    image->imageItem()->setHorizontalAlignment(static_cast<QQuickImage::HAlignment>(halign));
}

void DQuickIconLabelPrivate::updateOrSyncImage()
{
    if (updateImage()) {
        if (componentComplete) {
            updateImplicitSize();
            layout();
        }
    } else {
        syncImage();
    }
}

bool DQuickIconLabelPrivate::createLabel()
{
    Q_Q(DQuickIconLabel);
    if (label)
        return false;

    label = new QQuickText(q);
    watchChanges(label);
    beginClass(label);
    label->setObjectName(QStringLiteral("label"));
    label->setFont(font);
    label->setColor(color);
    label->setElideMode(QQuickText::ElideRight);
    const int valign = static_cast<int>(alignment & Qt::AlignVertical_Mask);
    label->setVAlign(static_cast<QQuickText::VAlignment>(valign));
    const int halign = static_cast<int>(alignment & Qt::AlignHorizontal_Mask);
    label->setHAlign(static_cast<QQuickText::HAlignment>(halign));
    label->setText(text);
    if (componentComplete)
        completeComponent(label);
    return true;
}

bool DQuickIconLabelPrivate::destroyLabel()
{
    if (!label)
        return false;

    unwatchChanges(label);
    delete label;
    label = nullptr;
    return true;
}

bool DQuickIconLabelPrivate::updateLabel()
{
    if (!hasText())
        return destroyLabel();
    return createLabel();
}

void DQuickIconLabelPrivate::syncLabel()
{
    if (!label)
        return;

    label->setText(text);
}

void DQuickIconLabelPrivate::updateOrSyncLabel()
{
    if (updateLabel()) {
        if (componentComplete) {
            updateImplicitSize();
            layout();
        }
    } else {
        syncLabel();
    }
}

void DQuickIconLabelPrivate::updateImplicitSize()
{
    Q_Q(DQuickIconLabel);
    const bool showIcon = image && hasIcon();
    const bool showText = label && hasText();
    const qreal horizontalPadding = leftPadding + rightPadding;
    const qreal verticalPadding = topPadding + bottomPadding;
    const qreal iconImplicitWidth = showIcon ? image->implicitWidth() : 0;
    const qreal iconImplicitHeight = showIcon ? image->implicitHeight() : 0;
    const qreal textImplicitWidth = showText ? label->implicitWidth() : 0;
    const qreal textImplicitHeight = showText ? label->implicitHeight() : 0;
    const qreal effectiveSpacing = showText && showIcon && image->implicitWidth() > 0 ? spacing : 0;
    const bool needAddIconAndTextWidth = display == DQuickIconLabel::TextBesideIcon || display == DQuickIconLabel::IconBesideText;
    const qreal implicitWidth = needAddIconAndTextWidth ? iconImplicitWidth + textImplicitWidth + effectiveSpacing
                                                                           : qMax(iconImplicitWidth, textImplicitWidth);
    const qreal implicitHeight = display == DQuickIconLabel::TextUnderIcon ? iconImplicitHeight + textImplicitHeight + effectiveSpacing
                                                                           : qMax(iconImplicitHeight, textImplicitHeight);
    q->setImplicitSize(implicitWidth + horizontalPadding, implicitHeight + verticalPadding);
}

// adapted from QStyle::alignedRect()
static QRectF alignedRect(bool mirrored, Qt::Alignment alignment, const QSizeF &size, const QRectF &rectangle)
{
    alignment = QGuiApplicationPrivate::visualAlignment(mirrored ? Qt::RightToLeft : Qt::LeftToRight, alignment);
    qreal x = rectangle.x();
    qreal y = rectangle.y();
    const qreal w = size.width();
    const qreal h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += rectangle.height() / 2 - h / 2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rectangle.height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rectangle.width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += rectangle.width() / 2 - w / 2;
    return QRectF(x, y, w, h);
}

void DQuickIconLabelPrivate::layout()
{
    Q_Q(DQuickIconLabel);
    if (!componentComplete)
        return;

    const qreal availableWidth = width - leftPadding - rightPadding;
    const qreal availableHeight = height - topPadding - bottomPadding;

    switch (display) {
    case DQuickIconLabel::IconOnly:
        if (image) {
            const QRectF iconRect = alignedRect(mirrored, alignment,
                                                QSizeF(qMin(image->implicitWidth(), availableWidth),
                                                       qMin(image->implicitHeight(), availableHeight)),
                                                QRectF(leftPadding, topPadding, availableWidth, availableHeight));
            image->setSize(iconRect.size());
            image->setPosition(iconRect.topLeft());
        }
        break;
    case DQuickIconLabel::TextOnly:
        if (label) {
            const QRectF textRect = alignedRect(mirrored, alignment,
                                                QSizeF(qMin(label->implicitWidth(), availableWidth),
                                                       qMin(label->implicitHeight(), availableHeight)),
                                                QRectF(leftPadding, topPadding, availableWidth, availableHeight));
            label->setSize(textRect.size());
            label->setPosition(textRect.topLeft());
        }
        break;

    case DQuickIconLabel::TextUnderIcon: {
        // Work out the sizes first, as the positions depend on them.
        QSizeF iconSize;
        QSizeF textSize;
        if (image) {
            iconSize.setWidth(qMin(image->implicitWidth(), availableWidth));
            iconSize.setHeight(qMin(image->implicitHeight(), availableHeight));
        }
        qreal effectiveSpacing = 0;
        if (label) {
            if (!iconSize.isEmpty())
                effectiveSpacing = spacing;
            textSize.setWidth(qMin(label->implicitWidth(), availableWidth));
            textSize.setHeight(qMin(label->implicitHeight(), availableHeight - iconSize.height() - effectiveSpacing));
        }

        QRectF combinedRect = alignedRect(mirrored, alignment,
                                          QSizeF(qMax(iconSize.width(), textSize.width()),
                                                 iconSize.height() + effectiveSpacing + textSize.height()),
                                          QRectF(leftPadding, topPadding, availableWidth, availableHeight));
        if (image) {
            QRectF iconRect = alignedRect(mirrored, Qt::Alignment(Qt::AlignHCenter | Qt::AlignTop), iconSize, combinedRect);
            image->setSize(iconRect.size());
            image->setPosition(iconRect.topLeft());
        }
        if (label) {
            QRectF textRect = alignedRect(mirrored, Qt::Alignment(Qt::AlignHCenter | Qt::AlignBottom), textSize, combinedRect);
            label->setSize(textRect.size());
            label->setPosition(textRect.topLeft());
        }
        break;
    }
    case DQuickIconLabel::IconBesideText: {
        // Work out the sizes first, as the positions depend on them.
        QSizeF iconSize(0, 0);
        QSizeF textSize(0, 0);
        if (image) {
            iconSize.setWidth(qMin(image->implicitWidth(), availableWidth));
            iconSize.setHeight(qMin(image->implicitHeight(), availableHeight));
        }
        qreal effectiveSpacing = 0;
        if (label) {
            if (!iconSize.isEmpty())
                effectiveSpacing = spacing;
            textSize.setWidth(qMin(label->implicitWidth(), availableWidth - iconSize.width() - effectiveSpacing));
            textSize.setHeight(qMin(label->implicitHeight(), availableHeight));
        }

        const QRectF combinedRect = alignedRect(mirrored, alignment,
                                                QSizeF(availableWidth,
                                                       qMax(iconSize.height(), textSize.height())),
                                                QRectF(leftPadding, topPadding, availableWidth, availableHeight));
        if (image) {
            const QRectF iconRect = alignedRect(mirrored, Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter), iconSize, combinedRect);
            image->setSize(iconRect.size());
            image->setPosition(iconRect.topLeft());
        }
        if (label) {
            const QRectF textRect = alignedRect(mirrored, Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter), textSize, combinedRect);
            label->setSize(textRect.size());
            label->setPosition(textRect.topLeft());
        }
        break;
    }

    case DQuickIconLabel::TextBesideIcon:
    default:
        // Work out the sizes first, as the positions depend on them.
        QSizeF iconSize(0, 0);
        QSizeF textSize(0, 0);
        if (image) {
            iconSize.setWidth(qMin(image->implicitWidth(), availableWidth));
            iconSize.setHeight(qMin(image->implicitHeight(), availableHeight));
        }
        qreal effectiveSpacing = 0;
        if (label) {
            if (!iconSize.isEmpty())
                effectiveSpacing = spacing;
            textSize.setWidth(qMin(label->implicitWidth(), availableWidth - iconSize.width() - effectiveSpacing));
            textSize.setHeight(qMin(label->implicitHeight(), availableHeight));
        }

        const QRectF combinedRect = alignedRect(mirrored, alignment,
                                                QSizeF(iconSize.width() + effectiveSpacing + textSize.width(),
                                                       qMax(iconSize.height(), textSize.height())),
                                                QRectF(leftPadding, topPadding, availableWidth, availableHeight));
        if (image) {
            const QRectF iconRect = alignedRect(mirrored, Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter), iconSize, combinedRect);
            image->setSize(iconRect.size());
            image->setPosition(iconRect.topLeft());
        }
        if (label) {
            const QRectF textRect = alignedRect(mirrored, Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter), textSize, combinedRect);
            label->setSize(textRect.size());
            label->setPosition(textRect.topLeft());
        }
        break;
    }

    q->setBaselineOffset(label ? label->y() + label->baselineOffset() : 0);
}

static const QQuickItemPrivate::ChangeTypes itemChangeTypes =
        QQuickItemPrivate::ImplicitWidth
        | QQuickItemPrivate::ImplicitHeight
        | QQuickItemPrivate::Destroyed;

void DQuickIconLabelPrivate::watchChanges(QQuickItem *item)
{
    QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(item);
    itemPrivate->addItemChangeListener(this, itemChangeTypes);
}

void DQuickIconLabelPrivate::unwatchChanges(QQuickItem* item)
{
    QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(item);
    itemPrivate->removeItemChangeListener(this, itemChangeTypes);
}

void DQuickIconLabelPrivate::itemImplicitWidthChanged(QQuickItem *)
{
    updateImplicitSize();
    layout();
}

void DQuickIconLabelPrivate::itemImplicitHeightChanged(QQuickItem *)
{
    updateImplicitSize();
    layout();
}

void DQuickIconLabelPrivate::itemDestroyed(QQuickItem *item)
{
    unwatchChanges(item);
    if (item == image)
        image = nullptr;
    else if (item == label)
        label = nullptr;
}

QSize DQuickIconLabelPrivate::iconSize() const
{
    D_QC(DQuickIconLabel);
    // If no size is specified for theme icons, it will use the smallest available size.
    QSize size(icon.width(), icon.height());
    if (size.width() <= 0)
        size.setWidth(q->width());
    if (size.height() <= 0)
        size.setHeight(q->height());
    return size;
}

DQuickIconLabel::DQuickIconLabel(QQuickItem *parent)
    : QQuickItem(*(new DQuickIconLabelPrivate), parent)
{
}

DQuickIconLabel::~DQuickIconLabel()
{
    Q_D(DQuickIconLabel);
    if (d->image)
        d->unwatchChanges(d->image);
    if (d->label)
        d->unwatchChanges(d->label);
}

DQuickDciIcon DQuickIconLabel::icon() const
{
    Q_D(const DQuickIconLabel);
    return d->icon;
}

void DQuickIconLabel::setIcon(const DQuickDciIcon &icon)
{
    Q_D(DQuickIconLabel);
    d->icon = icon;
    d->updateOrSyncImage();
}

QString DQuickIconLabel::text() const
{
    Q_D(const DQuickIconLabel);
    return d->text;
}

void DQuickIconLabel::setText(const QString &text)
{
    Q_D(DQuickIconLabel);
    if (d->text == text)
        return;

    d->text = text;
    d->updateOrSyncLabel();
}

QFont DQuickIconLabel::font() const
{
    Q_D(const DQuickIconLabel);
    return d->font;
}

void DQuickIconLabel::setFont(const QFont &font)
{
    Q_D(DQuickIconLabel);
    if (d->font == font)
        return;

    d->font = font;
    if (d->label)
        d->label->setFont(font);
}

QColor DQuickIconLabel::color() const
{
    Q_D(const DQuickIconLabel);
    return d->color;
}

void DQuickIconLabel::setColor(const QColor &color)
{
    Q_D(DQuickIconLabel);
    if (d->color == color)
        return;

    d->color = color;
    if (d->label)
        d->label->setColor(color);
}

DQuickIconLabel::Display DQuickIconLabel::display() const
{
    Q_D(const DQuickIconLabel);
    return d->display;
}

void DQuickIconLabel::setDisplay(Display display)
{
    Q_D(DQuickIconLabel);
    if (d->display == display)
        return;

    d->display = display;
    d->updateImage();
    d->updateLabel();
    d->updateImplicitSize();
    d->layout();
}

qreal DQuickIconLabel::spacing() const
{
    Q_D(const DQuickIconLabel);
    return d->spacing;
}

void DQuickIconLabel::setSpacing(qreal spacing)
{
    Q_D(DQuickIconLabel);
    if (qFuzzyCompare(d->spacing, spacing))
        return;

    d->spacing = spacing;
    if (d->image && d->label) {
        d->updateImplicitSize();
        d->layout();
    }
}

bool DQuickIconLabel::isMirrored() const
{
    Q_D(const DQuickIconLabel);
    return d->mirrored;
}

void DQuickIconLabel::setMirrored(bool mirrored)
{
    Q_D(DQuickIconLabel);
    if (d->mirrored == mirrored)
        return;

    d->mirrored = mirrored;
    d->layout();
}

Qt::Alignment DQuickIconLabel::alignment() const
{
    Q_D(const DQuickIconLabel);
    return d->alignment;
}

void DQuickIconLabel::setAlignment(Qt::Alignment alignment)
{
    Q_D(DQuickIconLabel);
    const int valign = static_cast<int>(alignment & Qt::AlignVertical_Mask);
    const int halign = static_cast<int>(alignment & Qt::AlignHorizontal_Mask);
    const uint align = static_cast<uint>((valign ? valign : Qt::AlignVCenter) | (halign ? halign : Qt::AlignHCenter));
    if (d->alignment == align)
        return;

    d->alignment = static_cast<Qt::Alignment>(align);
    if (d->label) {
        d->label->setVAlign(static_cast<QQuickText::VAlignment>(valign));
        d->label->setHAlign(static_cast<QQuickText::HAlignment>(halign));
    }
    if (d->image) {
        d->image->imageItem()->setVerticalAlignment(static_cast<QQuickImage::VAlignment>(valign));
        d->image->imageItem()->setHorizontalAlignment(static_cast<QQuickImage::HAlignment>(halign));
    }
    d->layout();
}

qreal DQuickIconLabel::topPadding() const
{
    Q_D(const DQuickIconLabel);
    return d->topPadding;
}

void DQuickIconLabel::setTopPadding(qreal padding)
{
    Q_D(DQuickIconLabel);
    if (qFuzzyCompare(d->topPadding, padding))
        return;

    d->topPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void DQuickIconLabel::resetTopPadding()
{
    setTopPadding(0);
}

qreal DQuickIconLabel::leftPadding() const
{
    Q_D(const DQuickIconLabel);
    return d->leftPadding;
}

void DQuickIconLabel::setLeftPadding(qreal padding)
{
    Q_D(DQuickIconLabel);
    if (qFuzzyCompare(d->leftPadding, padding))
        return;

    d->leftPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void DQuickIconLabel::resetLeftPadding()
{
    setLeftPadding(0);
}

qreal DQuickIconLabel::rightPadding() const
{
    Q_D(const DQuickIconLabel);
    return d->rightPadding;
}

void DQuickIconLabel::setRightPadding(qreal padding)
{
    Q_D(DQuickIconLabel);
    if (qFuzzyCompare(d->rightPadding, padding))
        return;

    d->rightPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void DQuickIconLabel::resetRightPadding()
{
    setRightPadding(0);
}

qreal DQuickIconLabel::bottomPadding() const
{
    Q_D(const DQuickIconLabel);
    return d->bottomPadding;
}

void DQuickIconLabel::setBottomPadding(qreal padding)
{
    Q_D(DQuickIconLabel);
    if (qFuzzyCompare(d->bottomPadding, padding))
        return;

    d->bottomPadding = padding;
    d->updateImplicitSize();
    d->layout();
}

void DQuickIconLabel::resetBottomPadding()
{
    setBottomPadding(0);
}

void DQuickIconLabel::componentComplete()
{
    Q_D(DQuickIconLabel);
    if (d->image)
        completeComponent(d->image);
    if (d->label)
        completeComponent(d->label);
    QQuickItem::componentComplete();
    d->layout();
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void DQuickIconLabel::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(DQuickIconLabel);
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    d->layout();
}
#else
void DQuickIconLabel::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(DQuickIconLabel);
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    d->layout();
}
#endif

DQUICK_END_NAMESPACE

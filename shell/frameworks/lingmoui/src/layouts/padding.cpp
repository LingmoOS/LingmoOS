/*
 *  SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "padding.h"

#include <QMarginsF>
#include <qnumeric.h>
#include <qtypes.h>

class PaddingPrivate
{
    Padding *const q;

public:
    enum Paddings {
        Left = 1 << 0,
        Top = 1 << 1,
        Right = 1 << 2,
        Bottom = 1 << 3,
        Horizontal = Left | Right,
        Vertical = Top | Bottom,
        All = Horizontal | Vertical
    };

    PaddingPrivate(Padding *qq)
        : q(qq)
    {
    }

    void calculateImplicitSize();
    void signalPaddings(const QMarginsF &oldPaddings, Paddings paddings);
    QMarginsF paddings() const;
    void disconnect();

    QPointer<QQuickItem> m_contentItem;

    qreal m_padding = 0;

    std::optional<qreal> m_horizontalPadding;
    std::optional<qreal> m_verticalPadding;

    std::optional<qreal> m_leftPadding;
    std::optional<qreal> m_topPadding;
    std::optional<qreal> m_rightPadding;
    std::optional<qreal> m_bottomPadding;
};

void PaddingPrivate::calculateImplicitSize()
{
    qreal impWidth = 0;
    qreal impHeight = 0;

    if (m_contentItem) {
        impWidth += m_contentItem->implicitWidth();
        impHeight += m_contentItem->implicitHeight();
    }

    impWidth += q->leftPadding() + q->rightPadding();
    impHeight += q->topPadding() + q->bottomPadding();

    q->setImplicitSize(impWidth, impHeight);
}

QMarginsF PaddingPrivate::paddings() const
{
    return {q->leftPadding(), q->topPadding(), q->rightPadding(), q->bottomPadding()};
}

void PaddingPrivate::signalPaddings(const QMarginsF &oldPaddings, Paddings which)
{
    if ((which & Left) && !qFuzzyCompare(q->leftPadding(), oldPaddings.left())) {
        Q_EMIT q->leftPaddingChanged();
    }
    if ((which & Top) && !qFuzzyCompare(q->topPadding(), oldPaddings.top())) {
        Q_EMIT q->topPaddingChanged();
    }
    if ((which & Right) && !qFuzzyCompare(q->rightPadding(), oldPaddings.right())) {
        Q_EMIT q->rightPaddingChanged();
    }
    if ((which & Bottom) && !qFuzzyCompare(q->bottomPadding(), oldPaddings.bottom())) {
        Q_EMIT q->bottomPaddingChanged();
    }
    if ((which == Horizontal || which == All)
        && (!qFuzzyCompare(q->leftPadding(), oldPaddings.left()) || !qFuzzyCompare(q->rightPadding(), oldPaddings.right()))) {
        Q_EMIT q->horizontalPaddingChanged();
    }
    if ((which == Vertical || which == All)
        && (!qFuzzyCompare(q->topPadding(), oldPaddings.top()) || !qFuzzyCompare(q->bottomPadding(), oldPaddings.bottom()))) {
        Q_EMIT q->verticalPaddingChanged();
    }
    if (!qFuzzyCompare(q->leftPadding() + q->rightPadding(), oldPaddings.left() + oldPaddings.right())) {
        Q_EMIT q->availableWidthChanged();
    }
    if (!qFuzzyCompare(q->topPadding() + q->bottomPadding(), oldPaddings.top() + oldPaddings.bottom())) {
        Q_EMIT q->availableHeightChanged();
    }
}

void PaddingPrivate::disconnect()
{
    if (m_contentItem) {
        QObject::disconnect(m_contentItem, &QQuickItem::implicitWidthChanged, q, &Padding::polish);
        QObject::disconnect(m_contentItem, &QQuickItem::implicitHeightChanged, q, &Padding::polish);
        QObject::disconnect(m_contentItem, &QQuickItem::visibleChanged, q, &Padding::polish);
        QObject::disconnect(m_contentItem, &QQuickItem::implicitWidthChanged, q, &Padding::implicitContentWidthChanged);
        QObject::disconnect(m_contentItem, &QQuickItem::implicitHeightChanged, q, &Padding::implicitContentHeightChanged);
    }
}

Padding::Padding(QQuickItem *parent)
    : QQuickItem(parent)
    , d(std::make_unique<PaddingPrivate>(this))
{
}

Padding::~Padding()
{
    d->disconnect();
}

void Padding::setContentItem(QQuickItem *item)
{
    if (d->m_contentItem == item) {
        return;
    }

    // Not hiding old contentItem unlike Control, because we can't reliably
    // restore it or force `visibile:` binding re-evaluation.
    if (d->m_contentItem) {
        d->disconnect();
        // Ideally, it should only unset the parent iff old item's parent is
        // `this`. But QtQuick.Controls/Control doesn't do that, and we don't
        // wanna even more inconsistencies with upstream.
        d->m_contentItem->setParentItem(nullptr);
    }

    d->m_contentItem = item;

    if (d->m_contentItem) {
        d->m_contentItem->setParentItem(this);
        connect(d->m_contentItem, &QQuickItem::implicitWidthChanged, this, &Padding::polish);
        connect(d->m_contentItem, &QQuickItem::implicitHeightChanged, this, &Padding::polish);
        connect(d->m_contentItem, &QQuickItem::visibleChanged, this, &Padding::polish);
        connect(d->m_contentItem, &QQuickItem::implicitWidthChanged, this, &Padding::implicitContentWidthChanged);
        connect(d->m_contentItem, &QQuickItem::implicitHeightChanged, this, &Padding::implicitContentHeightChanged);
    }

    polish();

    Q_EMIT contentItemChanged();
    Q_EMIT implicitContentWidthChanged();
    Q_EMIT implicitContentWidthChanged();
}

QQuickItem *Padding::contentItem()
{
    return d->m_contentItem;
}

void Padding::setPadding(qreal padding)
{
    if (qFuzzyCompare(padding, d->m_padding)) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_padding = padding;

    Q_EMIT paddingChanged();

    d->signalPaddings(oldPadding, PaddingPrivate::All);

    polish();
}

void Padding::resetPadding()
{
    if (qFuzzyCompare(d->m_padding, 0)) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_padding = 0;

    Q_EMIT paddingChanged();

    d->signalPaddings(oldPadding, PaddingPrivate::All);

    polish();
}

qreal Padding::padding() const
{
    return d->m_padding;
}

void Padding::setHorizontalPadding(qreal padding)
{
    if (qFuzzyCompare(padding, horizontalPadding()) && d->m_horizontalPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_horizontalPadding = padding;

    d->signalPaddings(oldPadding, PaddingPrivate::Horizontal);

    polish();
}

void Padding::resetHorizontalPadding()
{
    if (!d->m_horizontalPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_horizontalPadding.reset();

    d->signalPaddings(oldPadding, PaddingPrivate::Horizontal);

    polish();
}

qreal Padding::horizontalPadding() const
{
    return d->m_horizontalPadding.value_or(d->m_padding);
}

void Padding::setVerticalPadding(qreal padding)
{
    if (qFuzzyCompare(padding, verticalPadding()) && d->m_verticalPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_verticalPadding = padding;

    d->signalPaddings(oldPadding, PaddingPrivate::Vertical);

    polish();
}

void Padding::resetVerticalPadding()
{
    if (!d->m_verticalPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_verticalPadding.reset();

    d->signalPaddings(oldPadding, PaddingPrivate::Vertical);

    polish();
}

qreal Padding::verticalPadding() const
{
    return d->m_verticalPadding.value_or(d->m_padding);
}

void Padding::setLeftPadding(qreal padding)
{
    const QMarginsF oldPadding = d->paddings();
    if (qFuzzyCompare(padding, oldPadding.left()) && d->m_leftPadding.has_value()) {
        return;
    }

    d->m_leftPadding = padding;

    d->signalPaddings(oldPadding, PaddingPrivate::Left);

    polish();
}

void Padding::resetLeftPadding()
{
    if (!d->m_leftPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_leftPadding.reset();

    d->signalPaddings(oldPadding, PaddingPrivate::Left);

    polish();
}

qreal Padding::leftPadding() const
{
    if (d->m_leftPadding.has_value()) {
        return d->m_leftPadding.value();
    } else {
        return horizontalPadding();
    }
}

void Padding::setTopPadding(qreal padding)
{
    const QMarginsF oldPadding = d->paddings();
    if (qFuzzyCompare(padding, oldPadding.top()) && d->m_topPadding.has_value()) {
        return;
    }

    d->m_topPadding = padding;

    d->signalPaddings(oldPadding, PaddingPrivate::Top);

    polish();
}

void Padding::resetTopPadding()
{
    if (!d->m_topPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_topPadding.reset();

    d->signalPaddings(oldPadding, PaddingPrivate::Top);

    polish();
}

qreal Padding::topPadding() const
{
    if (d->m_topPadding.has_value()) {
        return d->m_topPadding.value();
    } else {
        return verticalPadding();
    }
}

void Padding::setRightPadding(qreal padding)
{
    const QMarginsF oldPadding = d->paddings();
    if (qFuzzyCompare(padding, oldPadding.right()) && d->m_rightPadding.has_value()) {
        return;
    }

    d->m_rightPadding = padding;

    d->signalPaddings(oldPadding, PaddingPrivate::Right);

    polish();
}

void Padding::resetRightPadding()
{
    if (!d->m_rightPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_rightPadding.reset();

    d->signalPaddings(oldPadding, PaddingPrivate::Right);

    polish();
}

qreal Padding::rightPadding() const
{
    if (d->m_rightPadding.has_value()) {
        return d->m_rightPadding.value();
    } else {
        return horizontalPadding();
    }
}

void Padding::setBottomPadding(qreal padding)
{
    const QMarginsF oldPadding = d->paddings();
    if (qFuzzyCompare(padding, oldPadding.bottom()) && d->m_bottomPadding.has_value()) {
        return;
    }

    d->m_bottomPadding = padding;

    d->signalPaddings(oldPadding, PaddingPrivate::Bottom);

    polish();
}

void Padding::resetBottomPadding()
{
    if (!d->m_bottomPadding.has_value()) {
        return;
    }

    const QMarginsF oldPadding = d->paddings();
    d->m_bottomPadding.reset();

    d->signalPaddings(oldPadding, PaddingPrivate::Bottom);

    polish();
}

qreal Padding::bottomPadding() const
{
    if (d->m_bottomPadding.has_value()) {
        return d->m_bottomPadding.value();
    } else {
        return verticalPadding();
    }
}

qreal Padding::availableWidth() const
{
    return width() - leftPadding() - rightPadding();
}

qreal Padding::availableHeight() const
{
    return height() - topPadding() - bottomPadding();
}

qreal Padding::implicitContentWidth() const
{
    if (d->m_contentItem) {
        return d->m_contentItem->implicitWidth();
    } else {
        return 0.0;
    }
}

qreal Padding::implicitContentHeight() const
{
    if (d->m_contentItem) {
        return d->m_contentItem->implicitHeight();
    } else {
        return 0.0;
    }
}

void Padding::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry != oldGeometry) {
        Q_EMIT availableWidthChanged();
        Q_EMIT availableHeightChanged();
        polish();
    }

    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

void Padding::updatePolish()
{
    d->calculateImplicitSize();
    if (!d->m_contentItem) {
        return;
    }

    d->m_contentItem->setPosition(QPointF(leftPadding(), topPadding()));
    d->m_contentItem->setSize(QSizeF(availableWidth(), availableHeight()));
}

void Padding::componentComplete()
{
    QQuickItem::componentComplete();
    // This is important! We must have a geometry so our parents can lay out.
    updatePolish();
}

#include "moc_padding.cpp"

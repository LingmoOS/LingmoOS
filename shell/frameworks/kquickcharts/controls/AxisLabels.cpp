/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "AxisLabels.h"

#include <QDebug>
#include <QQmlContext>

#include "ItemBuilder.h"
#include "datasource/ChartDataSource.h"

AxisLabelsAttached::AxisLabelsAttached(QObject *parent)
    : QObject(parent)
{
}

int AxisLabelsAttached::index() const
{
    return m_index;
}

void AxisLabelsAttached::setIndex(int newIndex)
{
    if (newIndex == m_index) {
        return;
    }

    m_index = newIndex;
    Q_EMIT indexChanged();
}

QString AxisLabelsAttached::label() const
{
    return m_label;
}

void AxisLabelsAttached::setLabel(const QString &newLabel)
{
    if (newLabel == m_label) {
        return;
    }

    m_label = newLabel;
    Q_EMIT labelChanged();
}

AxisLabels::AxisLabels(QQuickItem *parent)
    : QQuickItem(parent)
{
    m_itemBuilder = std::make_unique<ItemBuilder>();
    connect(m_itemBuilder.get(), &ItemBuilder::finished, this, &AxisLabels::scheduleLayout);
    connect(m_itemBuilder.get(), &ItemBuilder::beginCreate, this, &AxisLabels::onBeginCreate);
}

AxisLabels::~AxisLabels() = default;

AxisLabels::Direction AxisLabels::direction() const
{
    return m_direction;
}

void AxisLabels::setDirection(AxisLabels::Direction newDirection)
{
    if (newDirection == m_direction) {
        return;
    }

    m_direction = newDirection;
    scheduleLayout();
    Q_EMIT directionChanged();
}

QQmlComponent *AxisLabels::delegate() const
{
    return m_itemBuilder->component();
}

void AxisLabels::setDelegate(QQmlComponent *newDelegate)
{
    if (newDelegate == m_itemBuilder->component()) {
        return;
    }

    m_itemBuilder->setComponent(newDelegate);
    updateLabels();
    Q_EMIT delegateChanged();
}

ChartDataSource *AxisLabels::source() const
{
    return m_source;
}

void AxisLabels::setSource(ChartDataSource *newSource)
{
    if (newSource == m_source) {
        return;
    }

    if (m_source) {
        m_source->disconnect(this);
    }

    m_source = newSource;

    if (m_source) {
        connect(m_source, &ChartDataSource::dataChanged, this, [this]() {
            updateLabels();
        });
    }

    updateLabels();
    Q_EMIT sourceChanged();
}

Qt::Alignment AxisLabels::alignment() const
{
    return m_alignment;
}

void AxisLabels::setAlignment(Qt::Alignment newAlignment)
{
    if (newAlignment == m_alignment) {
        return;
    }

    m_alignment = newAlignment;
    scheduleLayout();
    Q_EMIT alignmentChanged();
}

bool AxisLabels::constrainToBounds() const
{
    return m_constrainToBounds;
}

void AxisLabels::setConstrainToBounds(bool newConstrainToBounds)
{
    if (newConstrainToBounds == m_constrainToBounds) {
        return;
    }

    m_constrainToBounds = newConstrainToBounds;
    scheduleLayout();
    Q_EMIT constrainToBoundsChanged();
}
void AxisLabels::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    if (newGeometry != oldGeometry) {
        scheduleLayout();
    }
}

void AxisLabels::scheduleLayout()
{
    if (!m_layoutScheduled) {
        auto scheduleLayoutLambda = [this]() {
            layout();
            m_layoutScheduled = false;
        };
        QMetaObject::invokeMethod(this, scheduleLayoutLambda, Qt::QueuedConnection);
        m_layoutScheduled = true;
    }
}

bool AxisLabels::isHorizontal()
{
    return m_direction == Direction::HorizontalLeftRight || m_direction == Direction::HorizontalRightLeft;
}

void AxisLabels::updateLabels()
{
    m_itemBuilder->clear();

    if (!m_itemBuilder->component() || !m_source) {
        return;
    }

    m_itemBuilder->setCount(m_source->itemCount());
    m_itemBuilder->build(this);
}

void AxisLabels::layout()
{
    if (!m_itemBuilder->isFinished()) {
        scheduleLayout();
        return;
    }

    auto maxWidth = 0.0;
    auto totalWidth = 0.0;
    auto maxHeight = 0.0;
    auto totalHeight = 0.0;

    auto labels = m_itemBuilder->items();
    for (auto label : labels) {
        maxWidth = std::max(maxWidth, label->width());
        maxHeight = std::max(maxHeight, label->height());
        totalWidth += label->width();
        totalHeight += label->height();
    }

    auto impWidth = isHorizontal() ? totalWidth : maxWidth;
    auto impHeight = isHorizontal() ? maxHeight : totalHeight;

    if (qFuzzyCompare(impWidth, width()) && qFuzzyCompare(impHeight, height())) {
        return;
    }

    setImplicitWidth(impWidth);
    setImplicitHeight(impHeight);

    auto spacing = (isHorizontal() ? width() : height()) / (labels.size() - 1);
    auto i = 0;
    auto layoutWidth = isHorizontal() ? 0.0 : width();
    auto layoutHeight = isHorizontal() ? height() : 0.0;

    for (auto label : labels) {
        auto x = 0.0;
        auto y = 0.0;

        switch (m_direction) {
        case Direction::HorizontalLeftRight:
            x = i * spacing;
            break;
        case Direction::HorizontalRightLeft:
            x = width() - i * spacing;
            break;
        case Direction::VerticalTopBottom:
            y = i * spacing;
            break;
        case Direction::VerticalBottomTop:
            y = height() - i * spacing;
            break;
        }

        if (m_alignment & Qt::AlignHCenter) {
            x += (layoutWidth - label->width()) / 2;
        } else if (m_alignment & Qt::AlignRight) {
            x += layoutWidth - label->width();
        }

        if (m_alignment & Qt::AlignVCenter) {
            y += (layoutHeight - label->height()) / 2;
        } else if (m_alignment & Qt::AlignBottom) {
            y += layoutHeight - label->height();
        }

        if (m_constrainToBounds) {
            x = std::max(x, 0.0);
            x = x + label->width() > width() ? width() - label->width() : x;
            y = std::max(y, 0.0);
            y = y + label->height() > height() ? height() - label->height() : y;
        }

        label->setX(x);
        label->setY(y);
        i++;
    }
}

void AxisLabels::onBeginCreate(int index, QQuickItem *item)
{
    QObject::connect(item, &QQuickItem::xChanged, this, [this]() {
        scheduleLayout();
    });
    QObject::connect(item, &QQuickItem::yChanged, this, [this]() {
        scheduleLayout();
    });
    QObject::connect(item, &QQuickItem::widthChanged, this, [this]() {
        scheduleLayout();
    });
    QObject::connect(item, &QQuickItem::heightChanged, this, [this]() {
        scheduleLayout();
    });

    auto attached = static_cast<AxisLabelsAttached *>(qmlAttachedPropertiesObject<AxisLabels>(item, true));
    attached->setIndex(index);
    attached->setLabel(m_source->item(index).toString());
}

#include "moc_AxisLabels.cpp"

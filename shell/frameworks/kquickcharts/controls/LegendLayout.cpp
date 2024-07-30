/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "LegendLayout.h"

#include <cmath>

#include "Chart.h"
#include "ItemBuilder.h"
#include "datasource/ChartDataSource.h"

qreal sizeWithSpacing(int count, qreal size, qreal spacing)
{
    return size * count + spacing * (count - 1);
}

LegendLayoutAttached::LegendLayoutAttached(QObject *parent)
    : QObject(parent)
{
}

qreal LegendLayoutAttached::minimumWidth() const
{
    return m_minimumWidth.value_or(0.0);
}

void LegendLayoutAttached::setMinimumWidth(qreal newMinimumWidth)
{
    if (newMinimumWidth == m_minimumWidth) {
        return;
    }

    m_minimumWidth = newMinimumWidth;
    Q_EMIT minimumWidthChanged();
}

bool LegendLayoutAttached::isMinimumWidthValid() const
{
    return m_minimumWidth.has_value();
}

qreal LegendLayoutAttached::preferredWidth() const
{
    return m_preferredWidth.value_or(0.0);
}

void LegendLayoutAttached::setPreferredWidth(qreal newPreferredWidth)
{
    if (newPreferredWidth == m_preferredWidth) {
        return;
    }

    m_preferredWidth = newPreferredWidth;
    Q_EMIT preferredWidthChanged();
}

bool LegendLayoutAttached::isPreferredWidthValid() const
{
    return m_preferredWidth.has_value();
}

qreal LegendLayoutAttached::maximumWidth() const
{
    return m_maximumWidth.value_or(0.0);
}

void LegendLayoutAttached::setMaximumWidth(qreal newMaximumWidth)
{
    if (newMaximumWidth == m_maximumWidth) {
        return;
    }

    m_maximumWidth = newMaximumWidth;
    Q_EMIT maximumWidthChanged();
}

bool LegendLayoutAttached::isMaximumWidthValid() const
{
    return m_maximumWidth.has_value();
}

LegendLayout::LegendLayout(QQuickItem *parent)
    : QQuickItem(parent)
{
}

qreal LegendLayout::horizontalSpacing() const
{
    return m_horizontalSpacing;
}

void LegendLayout::setHorizontalSpacing(qreal newHorizontalSpacing)
{
    if (newHorizontalSpacing == m_horizontalSpacing) {
        return;
    }

    m_horizontalSpacing = newHorizontalSpacing;
    polish();
    Q_EMIT horizontalSpacingChanged();
}

qreal LegendLayout::verticalSpacing() const
{
    return m_verticalSpacing;
}

void LegendLayout::setVerticalSpacing(qreal newVerticalSpacing)
{
    if (newVerticalSpacing == m_verticalSpacing) {
        return;
    }

    m_verticalSpacing = newVerticalSpacing;
    polish();
    Q_EMIT verticalSpacingChanged();
}

qreal LegendLayout::preferredWidth() const
{
    return m_preferredWidth;
}

void LegendLayout::componentComplete()
{
    QQuickItem::componentComplete();

    m_completed = true;
    polish();
}

void LegendLayout::updatePolish()
{
    if (!m_completed) {
        return;
    }

    int columns = 0;
    int rows = 0;
    qreal itemWidth = 0.0;
    qreal itemHeight = 0.0;

    qreal layoutWidth = width();

    std::tie(columns, rows, itemWidth, itemHeight) = determineColumns();

    auto column = 0;
    auto row = 0;

    const auto items = childItems();
    for (auto item : items) {
        if (!item->isVisible() || item->implicitWidth() <= 0 || item->implicitHeight() <= 0) {
            continue;
        }

        auto attached = static_cast<LegendLayoutAttached *>(qmlAttachedPropertiesObject<LegendLayout>(item, true));

        auto x = (itemWidth + m_horizontalSpacing) * column;
        auto y = (itemHeight + m_verticalSpacing) * row;

        item->setPosition(QPointF{x, y});
        item->setWidth(std::clamp(itemWidth, attached->minimumWidth(), attached->maximumWidth()));

        // If we are in single column mode, we are most likely width constrained.
        // In that case, we should make sure items do not exceed our own width,
        // so we can trigger things like text eliding.
        if (layoutWidth > 0 && item->width() > layoutWidth && columns == 1) {
            item->setWidth(layoutWidth);
        }

        column++;
        if (column >= columns) {
            row++;
            column = 0;
        }
    }

    setImplicitSize(sizeWithSpacing(columns, itemWidth, m_horizontalSpacing), sizeWithSpacing(rows, itemHeight, m_verticalSpacing));
}

void LegendLayout::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry != oldGeometry) {
        polish();
    }
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

void LegendLayout::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    if (change == QQuickItem::ItemVisibleHasChanged || change == QQuickItem::ItemSceneChange) {
        polish();
    }

    if (change == QQuickItem::ItemChildAddedChange) {
        auto item = data.item;

        connect(item, &QQuickItem::implicitWidthChanged, this, &LegendLayout::polish);
        connect(item, &QQuickItem::implicitHeightChanged, this, &LegendLayout::polish);
        connect(item, &QQuickItem::visibleChanged, this, &LegendLayout::polish);

        auto attached = static_cast<LegendLayoutAttached *>(qmlAttachedPropertiesObject<LegendLayout>(item, true));
        connect(attached, &LegendLayoutAttached::minimumWidthChanged, this, &LegendLayout::polish);
        connect(attached, &LegendLayoutAttached::preferredWidthChanged, this, &LegendLayout::polish);
        connect(attached, &LegendLayoutAttached::maximumWidthChanged, this, &LegendLayout::polish);

        polish();
    }

    if (change == QQuickItem::ItemChildRemovedChange) {
        auto item = data.item;

        item->disconnect(this);
        auto attached = static_cast<LegendLayoutAttached *>(qmlAttachedPropertiesObject<LegendLayout>(item, false));
        if (attached) {
            attached->disconnect(this);
        }

        polish();
    }

    QQuickItem::itemChange(change, data);
}

// Determine how many columns and rows should be used for placing items and how
// large each item should be.
std::tuple<int, int, qreal, qreal> LegendLayout::determineColumns()
{
    auto minWidth = -std::numeric_limits<qreal>::max();
    auto preferredWidth = -std::numeric_limits<qreal>::max();
    auto maxWidth = std::numeric_limits<qreal>::max();
    auto maxHeight = -std::numeric_limits<qreal>::max();

    const auto items = childItems();

    // Keep track of actual visual and visible items, since childItems() also
    // includes stuff like repeaters.
    auto itemCount = 0;

    // First, we determine the minimum, preferred and maximum width of all
    // items. These are determined from the attached object, or implicitWidth
    // for minimum size if minimumWidth has not been set.
    //
    // We also determine the maximum height of items so we do not need to do
    // that later.
    for (auto item : items) {
        if (!item->isVisible() || item->implicitWidth() <= 0 || item->implicitHeight() <= 0) {
            continue;
        }

        auto attached = static_cast<LegendLayoutAttached *>(qmlAttachedPropertiesObject<LegendLayout>(item, true));

        if (attached->isMinimumWidthValid()) {
            minWidth = std::max(minWidth, attached->minimumWidth());
        } else {
            minWidth = std::max(minWidth, item->implicitWidth());
        }

        if (attached->isPreferredWidthValid()) {
            preferredWidth = std::max(preferredWidth, attached->preferredWidth());
        }

        if (attached->isMaximumWidthValid()) {
            maxWidth = std::min(maxWidth, attached->maximumWidth());
        }

        maxHeight = std::max(maxHeight, item->implicitHeight());

        itemCount++;
    }

    if (itemCount == 0) {
        return std::make_tuple(0, 0, 0, 0);
    }

    auto availableWidth = width();
    // Check if we have a valid width. If we cannot even fit a horizontalSpacing
    // we cannot do anything with the width and most likely did not get a width
    // assigned, so come up with some reasonable default width.
    //
    // For the default, layout everything in a full row, using either maxWidth
    // for each item if we have it or minWidth if we do not.
    if (availableWidth <= m_horizontalSpacing) {
        if (maxWidth <= 0.0) {
            availableWidth = sizeWithSpacing(itemCount, minWidth, m_horizontalSpacing);
        } else {
            availableWidth = sizeWithSpacing(itemCount, maxWidth, m_horizontalSpacing);
        }
    }

    // If none of the items have a maximum width set, default to filling all
    // available space.
    if (maxWidth <= 0.0 || maxWidth >= std::numeric_limits<qreal>::max()) {
        maxWidth = availableWidth;
    }

    // Ensure we don't try to size things below their minimum size.
    if (maxWidth < minWidth) {
        maxWidth = minWidth;
    }

    if (preferredWidth != m_preferredWidth) {
        m_preferredWidth = preferredWidth;
        Q_EMIT preferredWidthChanged();
    }

    auto columns = 1;
    auto rows = itemCount;
    bool fit = true;

    // Calculate the actual number of rows and columns by trying to fit items
    // until we find the right number.
    while (true) {
        auto minTotalWidth = sizeWithSpacing(columns, minWidth, m_horizontalSpacing);
        auto maxTotalWidth = sizeWithSpacing(columns, maxWidth, m_horizontalSpacing);

        // If the minimum width is less than our width, but the maximum is
        // larger, we found a correct solution since we can resize the items to
        // fit within the provided bounds.
        if (minTotalWidth <= availableWidth && maxTotalWidth >= availableWidth) {
            break;
        }

        // As long as we have more space available than the items' max size,
        // decrease the number of rows and that way increase the number of
        // columns we use to place items - unless that results in no rows, as
        // that means we've reached a state where we simply have more space than
        // needed.
        if (maxTotalWidth < availableWidth) {
            rows--;
            if (rows >= 1) {
                columns = std::ceil(itemCount / float(rows));
            } else {
                fit = false;
                break;
            }
        }

        // In certain cases, we hit a corner case where decreasing the number of
        // rows leads to things ending up outside of the item's bounds. If that
        // happens, increase the number of rows by one and exit the loop.
        if (minTotalWidth > availableWidth) {
            rows += 1;
            columns = std::ceil(itemCount / float(rows));
            break;
        }
    }

    // Calculate item width based on the calculated number of columns.
    // If it turns out we have more space than needed, use maxWidth
    // instead to avoid awkward gaps.
    auto itemWidth = fit ? (availableWidth - m_horizontalSpacing * (columns - 1)) / columns : maxWidth;

    // Recalculate the number of rows, otherwise we may end up with "ghost" rows
    // since the items wrapped into a new column, but no all of them.
    rows = std::ceil(itemCount / float(columns));

    return std::make_tuple(columns, rows, itemWidth, maxHeight);
}

#include "moc_LegendLayout.cpp"

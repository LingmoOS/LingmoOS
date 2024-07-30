/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ColorGradientSource.h"

#include <QVariant>

#include <QDebug>

ColorGradientSource::ColorGradientSource(QObject *parent)
    : ChartDataSource(parent)
{
}

int ColorGradientSource::itemCount() const
{
    return m_itemCount;
}

QVariant ColorGradientSource::item(int index) const
{
    if (index < 0 || index >= m_colors.size()) {
        return QVariant{};
    }

    return m_colors.at(index);
}

QVariant ColorGradientSource::minimum() const
{
    return QVariant{};
}

QVariant ColorGradientSource::maximum() const
{
    return QVariant{};
}

QColor ColorGradientSource::baseColor() const
{
    return m_baseColor;
}

void ColorGradientSource::setBaseColor(const QColor &newBaseColor)
{
    if (newBaseColor == m_baseColor) {
        return;
    }

    m_baseColor = newBaseColor;
    regenerateColors();
    Q_EMIT baseColorChanged();
}

void ColorGradientSource::setItemCount(int newItemCount)
{
    if (newItemCount == m_itemCount) {
        return;
    }

    m_itemCount = newItemCount;
    regenerateColors();
    Q_EMIT itemCountChanged();
}

QVariantList ColorGradientSource::colors() const
{
    QVariantList colorsVariant;
    colorsVariant.reserve(m_colors.count());
    for (const QColor &color : std::as_const(m_colors)) {
        colorsVariant.append(color);
    }
    return colorsVariant;
}

void ColorGradientSource::regenerateColors()
{
    if (!m_baseColor.isValid() || m_itemCount <= 0) {
        return;
    }

    m_colors.clear();

    for (int i = 0; i < m_itemCount; ++i) {
        auto newHue = m_baseColor.hsvHueF() + i * (1.0 / m_itemCount);
        newHue = newHue - int(newHue);
        m_colors.append(QColor::fromHsvF(newHue, m_baseColor.saturationF(), m_baseColor.valueF(), m_baseColor.alphaF()));
    }

    Q_EMIT dataChanged();
}

#include "moc_ColorGradientSource.cpp"

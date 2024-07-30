/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef COLORGRADIENTSOURCE_H
#define COLORGRADIENTSOURCE_H

#include <QColor>
#include <QList>

#include "ChartDataSource.h"

/**
 * A data source that provides a hue-shifted color as data.
 */
class QUICKCHARTS_EXPORT ColorGradientSource : public ChartDataSource
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ColorGradientSource(QObject *parent = nullptr);

    Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    QColor baseColor() const;
    void setBaseColor(const QColor &newBaseColor);
    Q_SIGNAL void baseColorChanged();

    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)
    void setItemCount(int newItemCount);
    Q_SIGNAL void itemCountChanged();

    Q_PROPERTY(QVariantList colors READ colors NOTIFY dataChanged)
    QVariantList colors() const;

    int itemCount() const override;
    QVariant item(int index) const override;
    QVariant minimum() const override;
    QVariant maximum() const override;

private:
    void regenerateColors();

    QColor m_baseColor = Qt::blue;
    int m_itemCount = 0;
    QList<QColor> m_colors;
};

#endif // COLORGRADIENTSOURCE_H

/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ChartsPlugin.h"

#include "BarChart.h"
#include "Chart.h"
#include "LineChart.h"
#include "PieChart.h"
#include "RangeGroup.h"
#include "XYChart.h"

#include "decorations/AxisLabels.h"
#include "decorations/GridLines.h"
#include "decorations/LegendModel.h"
#include "decorations/LegendLayout.h"

#include "datasource/ArraySource.h"
#include "datasource/ChartAxisSource.h"
#include "datasource/ColorGradientSource.h"
#include "datasource/HistoryProxySource.h"
#include "datasource/MapProxySource.h"
#include "datasource/ModelSource.h"
#include "datasource/SingleValueSource.h"

#include "quickcharts_export.h"

QuickChartsPlugin::QuickChartsPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
{
}

void QuickChartsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString::fromLatin1(uri) == QLatin1String("org.kde.quickcharts"));

    qmlRegisterAnonymousType<QAbstractItemModel>(uri, 1);

    qmlRegisterType<PieChart>(uri, 1, 0, "PieChart");
    qmlRegisterType<LineChart>(uri, 1, 0, "LineChart");
    qmlRegisterType<BarChart>(uri, 1, 0, "BarChart");
    qmlRegisterUncreatableType<XYChart>(uri, 1, 0, "XYChart", QStringLiteral("Just a base class"));
    qmlRegisterUncreatableType<Chart>(uri, 1, 0, "Chart", QStringLiteral("Just a base class"));

    qmlRegisterUncreatableType<ChartDataSource>(uri, 1, 0, "ChartDataSource", QStringLiteral("Just a base class"));
    qmlRegisterType<ModelSource>(uri, 1, 0, "ModelSource");
    qmlRegisterType<SingleValueSource>(uri, 1, 0, "SingleValueSource");
    qmlRegisterType<ArraySource>(uri, 1, 0, "ArraySource");
    qmlRegisterType<ChartAxisSource>(uri, 1, 0, "ChartAxisSource");
    qmlRegisterType<ColorGradientSource>(uri, 1, 0, "ColorGradientSource");
    qmlRegisterType<MapProxySource>(uri, 1, 0, "MapProxySource");
    qmlRegisterType<HistoryProxySource>(uri, 1, 0, "HistoryProxySource");

    qmlRegisterUncreatableType<RangeGroup>(uri, 1, 0, "Range", QStringLiteral("Used as a grouped property"));

    qmlRegisterType<GridLines>(uri, 1, 0, "GridLines");
    qmlRegisterUncreatableType<LinePropertiesGroup>(uri, 1, 0, "LinePropertiesGroup", QStringLiteral("Used as a grouped property"));
    qmlRegisterType<AxisLabels>(uri, 1, 0, "AxisLabels");
    qmlRegisterUncreatableType<AxisLabelsAttached>(uri, 1, 0, "AxisLabelsAttached", QStringLiteral("Attached property"));
    qmlRegisterType<LegendModel>(uri, 1, 0, "LegendModel");
    qmlRegisterType<LegendLayout>(uri, 1, 0, "LegendLayout");
}

#include "moc_ChartsPlugin.cpp"

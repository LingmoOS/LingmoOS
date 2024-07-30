/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ChartsControlsPlugin.h"

#include <QDebug>
#include <QFile>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QUrl>

ChartsControlsPlugin::ChartsControlsPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
{
}

void ChartsControlsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString::fromLatin1(uri) == QLatin1String("org.kde.quickcharts.controls"));

    m_styleName = QQuickStyle::name();

    qmlRegisterSingletonType(componentUrl(QStringLiteral("Theme.qml")), uri, 1, 0, "Theme");
    qmlRegisterType(componentUrl(QStringLiteral("Legend.qml")), uri, 1, 0, "Legend");
    qmlRegisterType(componentUrl(QStringLiteral("LegendDelegate.qml")), uri, 1, 0, "LegendDelegate");
    qmlRegisterType(componentUrl(QStringLiteral("LineChartControl.qml")), uri, 1, 0, "LineChartControl");
    qmlRegisterType(componentUrl(QStringLiteral("PieChartControl.qml")), uri, 1, 0, "PieChartControl");

    qmlRegisterSingletonType(componentUrl(QStringLiteral("Logging.qml")), uri, 1, 0, "Logging");
}

QUrl ChartsControlsPlugin::componentUrl(const QString &fileName)
{
    auto url = baseUrl();
    url.setPath(url.path() % QLatin1Char('/'));

    auto styled = url.resolved(QUrl{QStringLiteral("styles/") % m_styleName % QLatin1Char('/') % fileName});
    if (QFile::exists(styled.toLocalFile())) {
        return styled;
    }

    return url.resolved(QUrl{fileName});
}

#include "moc_ChartsControlsPlugin.cpp"

/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CHARTSPLUGIN_H
#define CHARTSPLUGIN_H

#include <QQmlExtensionPlugin>

class QuickChartsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    explicit QuickChartsPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
};

#endif // CHARTSPLUGIN_H

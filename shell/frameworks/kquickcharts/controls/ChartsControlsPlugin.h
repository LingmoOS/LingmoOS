/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CHARTSCONTROLSPLUGIN_H
#define CHARTSCONTROLSPLUGIN_H

#include <QQmlExtensionPlugin>

class ChartsControlsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    explicit ChartsControlsPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;

private:
    QUrl componentUrl(const QString &fileName);
    QString m_styleName;
};

#endif // CHARTSCONTROLSPLUGIN_H

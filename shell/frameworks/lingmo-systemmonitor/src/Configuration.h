/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <memory>

#include <QObject>
#include <QQmlParserStatus>
#include <QTimer>

#include <qqmlregistration.h>

#include "systemmonitor.h"

/**
 * Wraps Config in an API similar to Qt.labs.settings
 *
 * This object should be used from QML. Any custom properties declared on the
 * QML instance will be forwarded from and to the KConfig based configuration
 * class.
 */
class Configuration : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit Configuration(QObject *parent = nullptr);

    Q_SLOT void propertyChanged();

    Q_SIGNAL void configurationLoaded();

    void classBegin() override;
    void componentComplete() override;

    static SystemMonitorConfiguration *globalConfig();

private:
    std::unique_ptr<QTimer> m_saveTimer;
};

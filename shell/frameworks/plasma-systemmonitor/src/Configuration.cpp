/*
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "Configuration.h"

#include <QDebug>
#include <QMetaProperty>
#include <chrono>

using namespace std::chrono_literals;

Q_GLOBAL_STATIC(SystemMonitorConfiguration, s_config)

SystemMonitorConfiguration *Configuration::globalConfig()
{
    return s_config;
}

Configuration::Configuration(QObject *parent)
    : QObject(parent)
    , m_saveTimer(std::make_unique<QTimer>())
{
    m_saveTimer->setInterval(500ms);
    m_saveTimer->setSingleShot(true);
    connect(m_saveTimer.get(), &QTimer::timeout, globalConfig(), &SystemMonitorConfiguration::save);
}

void Configuration::propertyChanged()
{
    auto signal = metaObject()->method(senderSignalIndex());
    // Strip "Changed" from the signal name to get property name
    auto propertyName = signal.name().chopped(7);
    auto property = metaObject()->property(metaObject()->indexOfProperty(propertyName));

    if (property.isValid()) {
        globalConfig()->setProperty(property.name(), property.read(this));
        m_saveTimer->start();
    } else {
        qWarning() << "Property" << propertyName << "was not found!";
    }
}

void Configuration::classBegin()
{
}

void Configuration::componentComplete()
{
    auto propertyChangedMethod = metaObject()->method(metaObject()->indexOfMethod("propertyChanged()"));

    for (auto i = 0; i < metaObject()->propertyCount(); ++i) {
        auto property = metaObject()->property(i);

        if (property.name() == QStringLiteral("config")) {
            continue;
        }

        if (globalConfig()->metaObject()->indexOfProperty(property.name()) == -1) {
            qWarning() << "Property" << property.name() << "not found in configuration, ignoring";
            continue;
        }

        property.write(this, globalConfig()->property(property.name()));

        if (property.hasNotifySignal()) {
            connect(this, property.notifySignal(), this, propertyChangedMethod);
        } else {
            qWarning() << "Property" << property.name() << "is not notifiable!";
        }
    }

    Q_EMIT configurationLoaded();
}

#include "moc_Configuration.cpp"

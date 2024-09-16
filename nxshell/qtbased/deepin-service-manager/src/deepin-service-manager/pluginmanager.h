// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "utils.h"

#include <QMap>
#include <QObject>

class PluginManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", PluginManagerInterface);
    Q_PROPERTY(QStringList Plugins READ plugins);

public:
    explicit PluginManager(QObject *parent = nullptr);

    void init(const QDBusConnection::BusType &type);
    void loadByGroup(const QString &group);
    void loadByName(const QString &name);

signals:
    Q_SCRIPTABLE void PluginAdded(const QString &plugin);
    Q_SCRIPTABLE void PluginRemoved(const QString &plugin);

private:
    QStringList plugins() const;

private:
    QStringList m_plugins;
    QDBusConnection::BusType m_type;
};

#endif // PLUGINMANAGER_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLUGINLOADER
#define PLUGINLOADER

#include <QDBusConnection>
#include <QMap>

class ServiceBase;
class Policy;
typedef QMap<QString, ServiceBase *> PluginMap;

class PluginLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList Plugins READ plugins);

public:
    explicit PluginLoader(QObject *parent = nullptr);
    ~PluginLoader();

    void init(const QDBusConnection::BusType &type, const bool isResident);
    void loadByGroup(const QString &group);
    void loadByName(const QString &name);
    QString getGroup(const QString &name);

signals:
    void PluginAdded(const QString &plugin);
    void PluginRemoved(const QString &plugin);

private:
    ServiceBase *createService(Policy *policy);
    void addPlugin(ServiceBase *obj);
    QList<Policy *> sortPolicy(QList<Policy *> policys);

    QStringList plugins() const;

private:
    PluginMap m_pluginMap;
    QDBusConnection::BusType m_type;
    bool m_isResident;
};

#endif // PLUGINLOADER

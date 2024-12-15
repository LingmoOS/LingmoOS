// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include "utils.h"

#include <QObject>

class GroupManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", ServiceGroupInterface);
    Q_PROPERTY(QStringList Plugins READ plugins);

public:
    explicit GroupManager(QObject *parent = nullptr);

public Q_SLOTS:
    void addPlugin(const QString &pluginName);
    void removePlugin(const QString &pluginName);
    void onNameOwnerChanged(const QString &service, const QString &from, const QString &to);

private:
    QStringList plugins() const;

private:
    QStringList m_plugins;
};

#endif // GROUPMANAGER_H

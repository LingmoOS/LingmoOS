// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>

#include "agent-extension.h"
#include "agent-extension-proxy.h"

using namespace dpa;

class QButtonGroup;
class PluginManager : public QObject, AgentExtensionProxy
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);

    QList<QButtonGroup*> reduceGetOptions(const QString &actionID);
    void reduce(const QString &username, const QString passwd);

    const QString & actionID() const Q_DECL_OVERRIDE { return m_actionID; }
    const QString & username() const Q_DECL_OVERRIDE { return m_username; }
    const QString & password() const Q_DECL_OVERRIDE { return m_password; }

    void setActionID(const QString &actionID);

private:
    void load();
    AgentExtension *loadFile(const QString &file);

private:
    QList<AgentExtension*> m_plugins;
    QString m_actionID;
    QString m_username;
    QString m_password;
};

#endif // PLUGINMANAGER_H

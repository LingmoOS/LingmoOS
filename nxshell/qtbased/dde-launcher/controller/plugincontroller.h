// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINCONTROLLER_H
#define PLUGINCONTROLLER_H

#include "plugininterface.h"
#include "pluginproxyinterface.h"
#include "common.h"

class LauncherPluginController : public QObject, public PluginProxyInterface
{
    Q_OBJECT
public:
    explicit LauncherPluginController(QObject *parent = Q_NULLPTR);
    ~LauncherPluginController() override {}

    void itemAdded(PluginInterface * const interface, const AppInfo &info) override;
    void itemRemoved(PluginInterface * const interface, const AppInfo &info) override;
    void itemUpdated(PluginInterface * const interface, const AppInfo &info) override;

    void startLoader();

Q_SIGNALS:
    void itemAddChanged(PluginInterface * const interface, const AppInfo &info);
    void itemUpdateChanged(PluginInterface * const interface, const AppInfo &info);
    void itemRemoveChanged(PluginInterface * const interface, const AppInfo &info);

public Q_SLOTS:
    void onSearchedTextChanged(const QString &keyword);

protected Q_SLOTS:
    void loadPlugin(const QString &pluginFile);
    void initPlugin(PluginInterface *interface);

private:
    QList<PluginInterface *> m_pluginAppInterList;
};

#endif

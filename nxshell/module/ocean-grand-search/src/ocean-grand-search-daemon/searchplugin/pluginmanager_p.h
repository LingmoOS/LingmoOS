// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_P_H
#define PLUGINMANAGER_P_H

#include "pluginmanager.h"
#include "loader/pluginloader.h"
#include "process/pluginprocess.h"

namespace GrandSearch {

class PluginManagerPrivate : public QObject
{
    Q_OBJECT
    friend class PluginManager;
public:
    explicit PluginManagerPrivate(PluginManager *parent);
    bool readConf();
    void prepareProcess();
signals:

public slots:
private:
    PluginManager *q;
    PluginLoader *m_loader = nullptr;
    PluginProcess *m_process = nullptr;
};

}

#endif // PLUGINMANAGER_P_H

// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "utils/searchplugininfo.h"

#include <QObject>

namespace GrandSearch {

class PluginManagerPrivate;
class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);

    bool loadPlugin();
    QList<SearchPluginInfo> plugins() const;
    void autoActivate();
signals:

public slots:
    bool activatePlugin(const QString &name);
    void inactivate(const QString &name);
private:
    PluginManagerPrivate *d;
};

}

#endif // PLUGINMANAGER_H

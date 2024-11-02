/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors:Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#ifndef FMWINDOWFACTORY_H
#define FMWINDOWFACTORY_H

#include <QObject>
#include "FMWindowIface.h"
#include "main-window-factory-iface.h"
#include <QMainWindow>

/*!
 * \brief The FMWindowFactoryPluginManager class
 * \deprecated
 */

class  MainWindowFactoryPluginManager : public QObject
{
public:
    static MainWindowFactoryPluginManager *getInstance();
    QString setVersion();
    QWidget *create(const QString &uri = nullptr);
    QWidget *create(const QStringList &uris);
    QWidget *create(const QString &uri, QStringList selectUris);
    const QStringList getPluginIds();
    QObject *getPlugin(const QString &pluginId);
    bool registerPlugin(Peony::MainWindowFactoryIface *plugin);
    void close();

protected:
    void loadAsync();

private:
    explicit MainWindowFactoryPluginManager(QObject *parent = nullptr);

    QHash<QString, Peony::MainWindowFactoryIface*> m_hash;
};


#endif // FMWINDOWFACTORY_H

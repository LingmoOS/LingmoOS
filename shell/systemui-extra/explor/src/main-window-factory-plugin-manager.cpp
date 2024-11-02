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
 * Authors: Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#include "main-window-factory-plugin-manager.h"
#include "main-window-factory-iface.h"
#include "main-window-factory.h"
#include "plugin-manager.h"
#ifdef LINGMO_COMMON
#include <lingmosdk/lingmo-com4cxx.h>
#endif
#include "global-settings.h"

#include <QDir>
#include <QtConcurrent>

MainWindowFactoryPluginManager *global_instance1 = nullptr;

MainWindowFactoryPluginManager *MainWindowFactoryPluginManager::getInstance()
{
    if (!global_instance1)
        global_instance1= new MainWindowFactoryPluginManager;
    return global_instance1;
}

QString MainWindowFactoryPluginManager::setVersion()
{
    int version = 4;
#ifdef LINGMO_COMMON
    version = Peony::GlobalSettings::getInstance()->getValue(PEONY_VERSION).toInt();
#endif //LINGMO_COMMON
    QString name = "lingmo4.0";
    switch(version) {
    case 3:
        name = "lingmo3.0";
        Peony::GlobalSettings::getInstance()->setValue(MULTI_SELECT, false);
        break;
    case 4:
    default:
        name = "lingmo4.0";
        Peony::GlobalSettings::getInstance()->setValue(MULTI_SELECT, true);
        break;
    };
    qApp->setProperty("version", name);
    return name;
}

QWidget *MainWindowFactoryPluginManager::create(const QString &uri)
{
    QString name = setVersion();
    Peony::MainWindowFactoryIface *iface = m_hash.value(name);
    QWidget *window = nullptr;
    if (iface) {
        window = iface->createWindow(uri);
    } else {
        window = m_hash.value("lingmo4.0")->createWindow(uri);
    }
    return window;
}

QWidget *MainWindowFactoryPluginManager::create(const QStringList &uris)
{
    if (uris.isEmpty()) {
        return create();
    }
    QWidget *window = create(uris.first());
    return window;
}

QWidget *MainWindowFactoryPluginManager::create(const QString &uri, QStringList selectUris)
{
    QString name = setVersion();
    Peony::MainWindowFactoryIface *iface = m_hash.value(name);
    QWidget *window = nullptr;
    if (iface) {
        window = iface->createWindow(uri);
    } else {
        window = m_hash.value("lingmo4.0")->createWindow(uri, selectUris);
    }
    return window;
}

MainWindowFactoryPluginManager::MainWindowFactoryPluginManager(QObject *parent) : QObject(parent)
{
    registerPlugin(MainWindowFactory::getInstance());

    QDir pluginsDir(WINDOW_PLUGIN_INSTALL_DIRS);
    pluginsDir.setFilter(QDir::Files);

    //QtConcurrent::run([=]() {
    qDebug()<<pluginsDir.entryList().count();
    Q_FOREACH(QString fileName, pluginsDir.entryList(QDir::Files)) {
        qDebug()<<fileName;

        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        qDebug()<<pluginLoader.fileName();
        qDebug()<<pluginLoader.metaData();
        qDebug()<<pluginLoader.load();

        // version check
        if (pluginLoader.metaData().value("MetaData").toObject().value("version").toString() != VERSION)
            continue;

        QObject *plugin = pluginLoader.instance();

        if (!plugin) {
            qWarning() << "load desktop error:" <<pluginLoader.errorString();
            continue;
        }
        Peony::MainWindowFactoryIface *piface = dynamic_cast<Peony::MainWindowFactoryIface*>(plugin);
                    qWarning() << "load desktop error:" <<pluginLoader.errorString();
        if (!piface)
            continue;
        qDebug()<<"load "<<piface->name();
        registerPlugin(piface);

    }

    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (PEONY_VERSION == key) {
            setVersion();
        }
    });
}

bool MainWindowFactoryPluginManager::registerPlugin(Peony::MainWindowFactoryIface *plugin)
{
    if (m_hash.value(plugin->name())) {
        return false;
    }
    m_hash.insert(plugin->name(), plugin);
    return true;
}

const QStringList MainWindowFactoryPluginManager::getPluginIds()
{
    return m_hash.keys();
}

QObject *MainWindowFactoryPluginManager::getPlugin(const QString &pluginId)
{
    auto iface = m_hash.value(pluginId);
    auto obj = dynamic_cast<QObject*>(iface);
    return obj;
}

/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "properties-window-factory-plugin-manager.h"
#include "properties-window-factory-plugin-iface.h"
#include "properties-window-factory.h"
#include <QApplication>
#include <QDir>
#include <QPluginLoader>
#include <QDebug>

using namespace Peony;

static PropertiesWindowFactoryPluginManager *global_instance = nullptr;

PropertiesWindowFactoryPluginManager *PropertiesWindowFactoryPluginManager::getInstance()
{
    if (!global_instance)
        global_instance = new PropertiesWindowFactoryPluginManager;
    return global_instance;
}


void PropertiesWindowFactoryPluginManager::release()
{
    deleteLater();
}

PropertiesWindowFactoryPluginManager::PropertiesWindowFactoryPluginManager(QObject *parent) : QObject(parent)
{
    //register internal factories.
    loadFactory(PropertiesWindowFactory::getInstance());

    QDir pluginsDir(PROPERTIES_WINDOW_PLUGIN_INSTALL_DIRS);
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
        Peony::PropertiesWindowFactoryPluginIface *piface = dynamic_cast<Peony::PropertiesWindowFactoryPluginIface*>(plugin);
        if (!piface)
            continue;
        qDebug()<<"load "<<piface->name();
        loadFactory(piface);

    }
}

PropertiesWindowFactoryPluginManager::~PropertiesWindowFactoryPluginManager()
{
    for (auto factory : m_factory_hash) {
        factory->closeFactory();
    }
    m_factory_hash.clear();
}

bool PropertiesWindowFactoryPluginManager::loadFactory(PropertiesWindowFactoryPluginIface *factory)
{
    auto id = factory->name();
    if (m_factory_hash.value(id)) {
        return false;
    }

    m_factory_hash.insert(id, factory);
    return true;
}

bool PropertiesWindowFactoryPluginManager::deleteFactory(PropertiesWindowFactoryPluginIface *factory)
{
    auto id = factory->name();
    if (m_factory_hash.value(id)) {
        m_factory_hash.remove(id);
        return true;
    }

    return false;
}

bool PropertiesWindowFactoryPluginManager::registerFactory(PropertiesWindowTabPagePluginIface *factory)
{
    QString version = qApp->property("version").toString();
    if (version == "") {
        version = "lingmo4.0";
    }
    PropertiesWindowFactoryPluginIface *iface = getFactory(version);
    QObject *obj = dynamic_cast<QObject*>(factory);
    iface->registerFactory(obj);
    return true;
}

bool PropertiesWindowFactoryPluginManager::unregisterFactory(PropertiesWindowTabPagePluginIface *factory)
{
    QString version = qApp->property("version").toString();
    if (version == "") {
        version = "lingmo4.0";
    }
    PropertiesWindowFactoryPluginIface *iface = getFactory(version);
    QObject *obj = dynamic_cast<QObject*>(factory);
    iface->unregisterFactory(obj);
    return false;
}

QMainWindow *PropertiesWindowFactoryPluginManager::create(const QStringList &uris)
{
    QString version = qApp->property("version").toString();
    if (version == "") {
        version = "lingmo4.0";
    }
    PropertiesWindowFactoryPluginIface *iface = getFactory(version);
    return iface->create(uris);
}

PropertiesWindowFactoryPluginIface *PropertiesWindowFactoryPluginManager::getFactory(const QString &id)
{
    PropertiesWindowFactoryPluginIface *factory = m_factory_hash.value(id);
    if (!factory) {
        factory = PropertiesWindowFactory::getInstance();
    }
    return factory;
}

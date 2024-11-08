/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "plugin-manager.h"

#include "menu-plugin-manager.h"
#include "directory-view-factory-manager.h"
#include "preview-page-factory-manager.h"

#include "directory-view-plugin-iface.h"
#include "directory-view-plugin-iface2.h"
#include "preview-page-plugin-iface.h"
#include "style-plugin-iface.h"
#include "vfs-plugin-manager.h"
#include "emblem-plugin-iface.h"
#include "vfs-info-plugin-iface.h"
#include "vfs-info-plugin-manager.h"

//#include "properties-window.h" //properties factory manager define is in this header
#include "properties-window-tab-page-plugin-iface.h"
#include "properties-window-factory-plugin-manager.h"
#include "directory-view-widget.h"

#include "global-settings.h"
#include "file-watcher.h"

#ifdef LINGMO_SDK_SYSINFO
#include <lingmosdk/lingmosdk-system/libkysysinfo.h>
#endif

#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QApplication>
#include <QProxyStyle>

using namespace Peony;

static PluginManager *global_instance = nullptr;

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{
    //FIXME: we have to ensure that internal factory being registered successfully.
    Peony::PropertiesWindowFactoryPluginManager::getInstance();
    //PropertiesWindowPluginManager::getInstance();
    MenuPluginManager::getInstance();
    DirectoryViewFactoryManager2::getInstance();
    PreviewPageFactoryManager::getInstance();
    VFSPluginManager::getInstance();
    VFSInfoPluginManager::getInstance();

    QDir pluginsDir(PLUGIN_INSTALL_DIRS);
//    if (COMMERCIAL_VERSION)
//        pluginsDir = QDir("/usr/lib/explorer-qt-extensions");
    pluginsDir.setFilter(QDir::Files);
    QStringList disabledExtensions = GlobalSettings::getInstance()->getValue(DISABLED_EXTENSIONS).toStringList();

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
        if (!plugin)
            continue;
        qDebug()<<"test start";
        PluginInterface *piface = dynamic_cast<PluginInterface*>(plugin);
        if (!piface)
            continue;

        QFileInfo fileInfo(pluginLoader.fileName());
        if (fileInfo.exists()) {
            if (disabledExtensions.contains(fileInfo.fileName())
                    && piface->pluginType() != PluginInterface::VFSPlugin) {
                continue;
            } else if (disabledExtensions.contains(fileInfo.fileName())
                       && piface->pluginType() == PluginInterface::VFSPlugin) {
                m_hash.insert(piface->name(), piface);
            } else if (!disabledExtensions.contains(fileInfo.fileName())
                       && piface->pluginType() != PluginInterface::VFSPlugin) {
                m_hash.insert(piface->name(), piface);
            }
        }

        m_fileNameHash.insert(fileInfo.fileName(), piface);
        switch (piface->pluginType()) {
        case PluginInterface::MenuPlugin: {
            MenuPluginInterface *menuPlugin = dynamic_cast<MenuPluginInterface*>(piface);
            MenuPluginManager::getInstance()->registerPlugin(menuPlugin);
            if ("libexplorer-drive-rename.so" == fileInfo.fileName()) {
                qApp->setProperty("deviceRenamePluginLoaded", true);
            }
            break;
        }
        case PluginInterface::PreviewPagePlugin: {
            PreviewPagePluginIface *previewPageFactory = dynamic_cast<PreviewPagePluginIface*>(plugin);
            PreviewPageFactoryManager::getInstance()->registerFactory(previewPageFactory->name(), previewPageFactory);
            break;
        }
        case PluginInterface::PropertiesWindowPlugin: {
            Peony::PropertiesWindowFactoryPluginManager *manager = Peony::PropertiesWindowFactoryPluginManager::getInstance();
            PropertiesWindowTabPagePluginIface *propertiesWindowTabPageFactory = dynamic_cast<PropertiesWindowTabPagePluginIface*>(plugin);
            manager->registerFactory(propertiesWindowTabPageFactory);
            break;
        }
        case PluginInterface::ColumnProviderPlugin: {
            //FIXME:
            break;
        }
        case  PluginInterface::StylePlugin: {
            /*!
              \todo
              manage the style plugin
              */
            auto styleProvider = dynamic_cast<StylePluginIface*>(plugin);
            QApplication::setStyle(styleProvider->getStyle());
            break;
        }
        case PluginInterface::DirectoryViewPlugin2: {
            auto p = dynamic_cast<DirectoryViewPluginIface2*>(plugin);
            DirectoryViewFactoryManager2::getInstance()->registerFactory(p->viewIdentity(), p);
            break;
        }
        case PluginInterface::VFSPlugin: {
            auto p = dynamic_cast<VFSPluginIface *>(plugin);
#ifdef LINGMO_SDK_SYSINFO
            if (p->name() == "file-safe vfs") {
                g_autofree char *isCloudPlat = kdk_system_get_hostVirtType();
                if (isCloudPlat != nullptr) {
                    qDebug() << "isCloudPlat is " << isCloudPlat;
                    if (strcmp(isCloudPlat, "none") == 0) {
                        VFSPluginManager::getInstance()->registerPlugin(p);
                    }
                    //delete isCloudPlat;
                }
            } else {
                VFSPluginManager::getInstance()->registerPlugin(p);
            }
#else
            VFSPluginManager::getInstance()->registerPlugin(p);
#endif
            break;
        }
        case PluginInterface::EmblemPlugin: {
            auto p = dynamic_cast<EmblemPluginInterface *>(plugin);
            EmblemProviderManager::getInstance()->registerProvider(p->create());
            break;
        }
        case PluginInterface::VFSINFOPlugin: {
            auto p = dynamic_cast<VFSInfoPluginIface *>(plugin);
            VFSInfoPluginManager::getInstance()->registerPlugin(p);
            break;
        }
        default:
            break;
        }
        registerPlugin(piface, plugin);
    }

    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=](const QString &key){
       if (DISABLED_EXTENSIONS == key) {
           QStringList disExtensions = GlobalSettings::getInstance()->getValue(key).toStringList();
           QDir pluginsDir(PLUGIN_INSTALL_DIRS);
       //    if (COMMERCIAL_VERSION)
       //        pluginsDir = QDir("/usr/lib/explorer-qt-extensions");
           pluginsDir.setFilter(QDir::Files);

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
               if (!plugin)
                   continue;
               qDebug()<<"test start";
               PluginInterface *piface = dynamic_cast<PluginInterface*>(plugin);
               if (!piface)
                   continue;

               QFileInfo fileInfo(pluginLoader.fileName());
               if (fileInfo.exists()) {
                   if (disExtensions.contains(fileInfo.fileName()) && m_hash.keys().contains(piface->name())
                           && piface->pluginType() != PluginInterface::VFSPlugin) {
                        m_hash.remove(piface->name());
                        m_fileNameHash.remove(fileInfo.fileName());
                        switch (piface->pluginType()) {
                        case PluginInterface::MenuPlugin: {
                           MenuPluginInterface *menuPlugin = dynamic_cast<MenuPluginInterface*>(piface);
                           //MenuPluginManager::getInstance()->registerPlugin(menuPlugin);
                           MenuPluginManager::getInstance()->unregisterPlugin(menuPlugin);
                           if ("libexplorer-drive-rename.so" == fileInfo.fileName()) {
                               qApp->setProperty("deviceRenamePluginLoaded", false);
                           }
                           break;
                        }
                        case PluginInterface::PreviewPagePlugin: {
                           PreviewPagePluginIface *previewPageFactory = dynamic_cast<PreviewPagePluginIface*>(plugin);
                           //PreviewPageFactoryManager::getInstance()->registerFactory(previewPageFactory->name(), previewPageFactory);
                           PreviewPageFactoryManager::getInstance()->unregisterFactory(previewPageFactory->name(), previewPageFactory);
                           break;
                        }
                        case PluginInterface::PropertiesWindowPlugin: {
                            Peony::PropertiesWindowFactoryPluginManager *manager = Peony::PropertiesWindowFactoryPluginManager::getInstance();
                            PropertiesWindowTabPagePluginIface *propertiesWindowTabPageFactory = dynamic_cast<PropertiesWindowTabPagePluginIface*>(plugin);
                            manager->unregisterFactory(propertiesWindowTabPageFactory);
                           break;
                        }
                        case PluginInterface::ColumnProviderPlugin: {
                           //FIXME:
                           break;
                        }
                        case PluginInterface::DirectoryViewPlugin2: {
                           auto p = dynamic_cast<DirectoryViewPluginIface2*>(plugin);
                           //DirectoryViewFactoryManager2::getInstance()->registerFactory(p->viewIdentity(), p);
                           DirectoryViewFactoryManager2::getInstance()->unregisterFactory(p->viewIdentity(), p);
                           break;
                        }
                        case PluginInterface::VFSPlugin: {
                           auto p = dynamic_cast<VFSPluginIface *>(plugin);
                           //VFSPluginManager::getInstance()->registerPlugin(p);
                           VFSPluginManager::getInstance()->unregisterPlugin(p);
                           break;
                        }
                        case PluginInterface::EmblemPlugin: {
                           auto p = dynamic_cast<EmblemPluginInterface *>(plugin);
                           //EmblemProviderManager::getInstance()->registerProvider(p->create());
                           EmblemProviderManager::getInstance()->unregisterProvider(p->create());
                           break;
                        }
                        default:
                           break;
                        }
                   } else if (!disExtensions.contains(fileInfo.fileName()) && !m_hash.keys().contains(piface->name())
                              && piface->pluginType() != PluginInterface::VFSPlugin) {
                        m_hash.insert(piface->name(), piface);
                        m_fileNameHash.insert(fileInfo.fileName(), piface);
                        switch (piface->pluginType()) {
                        case PluginInterface::MenuPlugin: {
                           MenuPluginInterface *menuPlugin = dynamic_cast<MenuPluginInterface*>(piface);
                           MenuPluginManager::getInstance()->registerPlugin(menuPlugin);
                           if ("libexplorer-drive-rename.so" == fileInfo.fileName()) {
                               qApp->setProperty("deviceRenamePluginLoaded", true);
                           }
                           break;
                        }
                        case PluginInterface::PreviewPagePlugin: {
                           PreviewPagePluginIface *previewPageFactory = dynamic_cast<PreviewPagePluginIface*>(plugin);
                           PreviewPageFactoryManager::getInstance()->registerFactory(previewPageFactory->name(), previewPageFactory);
                           break;
                        }
                        case PluginInterface::PropertiesWindowPlugin: {
                            Peony::PropertiesWindowFactoryPluginManager *manager = Peony::PropertiesWindowFactoryPluginManager::getInstance();
                            PropertiesWindowTabPagePluginIface *propertiesWindowTabPageFactory = dynamic_cast<PropertiesWindowTabPagePluginIface*>(plugin);
                            manager->registerFactory(propertiesWindowTabPageFactory);
                           break;
                        }
                        case PluginInterface::ColumnProviderPlugin: {
                           //FIXME:
                           break;
                        }
                        case  PluginInterface::StylePlugin: {
                           /*!
                             \todo
                             manage the style plugin
                             */
                           auto styleProvider = dynamic_cast<StylePluginIface*>(plugin);
                           QApplication::setStyle(styleProvider->getStyle());
                           break;
                        }
                        case PluginInterface::DirectoryViewPlugin2: {
                           auto p = dynamic_cast<DirectoryViewPluginIface2*>(plugin);
                           DirectoryViewFactoryManager2::getInstance()->registerFactory(p->viewIdentity(), p);
                           break;
                        }
                        case PluginInterface::VFSPlugin: {
                           auto p = dynamic_cast<VFSPluginIface *>(plugin);
                           //VFSPluginManager::getInstance()->registerPlugin(p);
                           Q_EMIT VFSPluginManager::getInstance()->updateVFSPlugin(p, true);
                           break;
                        }
                        case PluginInterface::EmblemPlugin: {
                           auto p = dynamic_cast<EmblemPluginInterface *>(plugin);
                           EmblemProviderManager::getInstance()->registerProvider(p->create());
                           break;
                        }
                        default:
                           break;
                        }
                   } else if (disExtensions.contains(fileInfo.fileName())
                              && piface->pluginType() == PluginInterface::VFSPlugin
                              && !m_hash.contains(piface->name())) {
                        m_hash.insert(piface->name(), piface);
                        auto p = dynamic_cast<VFSPluginIface *>(plugin);
                        Q_EMIT VFSPluginManager::getInstance()->updateVFSPlugin(p, true);
                   } else if (!disExtensions.contains(fileInfo.fileName())
                              && piface->pluginType() == PluginInterface::VFSPlugin
                              && m_hash.contains(piface->name())) {
                        m_hash.remove(piface->name());
                        auto p = dynamic_cast<VFSPluginIface *>(plugin);
                        Q_EMIT VFSPluginManager::getInstance()->updateVFSPlugin(p, false);
                   }
               }
           }
       }
     });

    auto pluginsDirWatcher = new Peony::FileWatcher(QString("file://%1").arg(PLUGIN_INSTALL_DIRS), this, true);
    pluginsDirWatcher->connect(pluginsDirWatcher, &Peony::FileWatcher::fileCreated, this, [=](const QString &uri){
        QUrl url = uri;
        QPluginLoader pluginLoader(url.path());

        // version check
        if (pluginLoader.metaData().value("MetaData").toObject().value("version").toString() != VERSION)
            return;

        QObject *plugin = pluginLoader.instance();
        if (!plugin)
            return;

        PluginInterface *piface = dynamic_cast<PluginInterface*>(plugin);
        if (!piface)
            return;

        m_hash.insert(piface->name(), piface);
        m_fileNameHash.insert(piface->name(), piface);
        registerPlugin(piface, plugin);
    });
    pluginsDirWatcher->startMonitor();
}

PluginManager::~PluginManager()
{
    m_hash.clear();
    MenuPluginManager::getInstance()->close();
    //FIXME: use private deconstructor.
    DirectoryViewFactoryManager2::getInstance()->deleteLater();
    PreviewPageFactoryManager::getInstance()->deleteLater();
}

PluginManager *PluginManager::getInstance()
{
    if (!global_instance) {
        global_instance = new PluginManager;
    }
    return global_instance;
}

void PluginManager::setPluginEnableByName(const QString &name, bool enable)
{
    m_hash.value(name)->setEnable(enable);
}

void PluginManager::close()
{
    if (global_instance)
        global_instance->deleteLater();
}

PluginInterface *PluginManager::getPluginByFileName(QString &fileName)
{
    if (!m_fileNameHash.contains(fileName)) {
        return nullptr;
    }
    return m_fileNameHash.value(fileName);
}

void PluginManager::registerPlugin(PluginInterface *piface, QObject *plugin)
{
    switch (piface->pluginType()) {
    case PluginInterface::MenuPlugin: {
        MenuPluginInterface *menuPlugin = dynamic_cast<MenuPluginInterface*>(piface);
        MenuPluginManager::getInstance()->registerPlugin(menuPlugin);
        break;
    }
    case PluginInterface::PreviewPagePlugin: {
        PreviewPagePluginIface *previewPageFactory = dynamic_cast<PreviewPagePluginIface*>(plugin);
        PreviewPageFactoryManager::getInstance()->registerFactory(previewPageFactory->name(), previewPageFactory);
        break;
    }
    case PluginInterface::PropertiesWindowPlugin: {
        Peony::PropertiesWindowFactoryPluginManager *manager = Peony::PropertiesWindowFactoryPluginManager::getInstance();
        PropertiesWindowTabPagePluginIface *propertiesWindowTabPageFactory = dynamic_cast<PropertiesWindowTabPagePluginIface*>(plugin);
        manager->registerFactory(propertiesWindowTabPageFactory);
        break;
    }
    case PluginInterface::ColumnProviderPlugin: {
        //FIXME:
        break;
    }
    case  PluginInterface::StylePlugin: {
        /*!
          \todo
          manage the style plugin
          */
        auto styleProvider = dynamic_cast<StylePluginIface*>(plugin);
        QApplication::setStyle(styleProvider->getStyle());
        break;
    }
    case PluginInterface::DirectoryViewPlugin2: {
        auto p = dynamic_cast<DirectoryViewPluginIface2*>(plugin);
        DirectoryViewFactoryManager2::getInstance()->registerFactory(p->viewIdentity(), p);
        break;
    }
    case PluginInterface::VFSPlugin: {
        auto p = dynamic_cast<VFSPluginIface *>(plugin);
        VFSPluginManager::getInstance()->registerPlugin(p);
        break;
    }
    case PluginInterface::EmblemPlugin: {
        auto p = dynamic_cast<EmblemPluginInterface *>(plugin);
        EmblemProviderManager::getInstance()->registerProvider(p->create());
        break;
    }
    default:
        break;
    }
}

void PluginManager::init()
{
    PluginManager::getInstance();
}

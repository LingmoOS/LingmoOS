// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugincontroller.h"
#include "pluginloader.h"
#include "appsmanager.h"

#include <QtConcurrent>

LauncherPluginController::LauncherPluginController(QObject *parent)
    : QObject (parent)
{
}

void LauncherPluginController::itemAdded(PluginInterface * const interface, const AppInfo &info)
{
    if (!interface)
        return;

    // sender = PluginProxyInterface
    emit itemAddChanged(interface, info);
}

void LauncherPluginController::itemRemoved(PluginInterface * const interface, const AppInfo &info)
{
    if (!interface)
        return;

    // sender = PluginProxyInterface
    emit itemRemoveChanged(interface, info);
}

void LauncherPluginController::itemUpdated(PluginInterface * const interface, const AppInfo &info)
{
    if (!interface)
        return;

    // sender = PluginProxyInterface
    emit itemUpdateChanged(interface, info);
}

void LauncherPluginController::startLoader()
{
    PluginLoader *pluginLoader = new PluginLoader;
    connect(pluginLoader, &PluginLoader::finished, pluginLoader, &PluginLoader::deleteLater, Qt::QueuedConnection);
    connect(pluginLoader, &PluginLoader::pluginFounded, this, &LauncherPluginController::loadPlugin, Qt::QueuedConnection);

    QTimer::singleShot(0, pluginLoader, [ = ] { pluginLoader->start(QThread::LowestPriority); });
}

void LauncherPluginController::loadPlugin(const QString &pluginFile)
{
    QPluginLoader *pluginLoader = new QPluginLoader(pluginFile, this);
    const QJsonObject &meta = pluginLoader->metaData().value("MetaData").toObject();
    const QString &pluginApi = meta.value("api").toString();
    if (pluginApi.isEmpty() || !CompatiblePluginApiList.contains(pluginApi)) {
        qDebug() << "api is empty or plugin is not compatible";
        return;
    }

    PluginInterface *interface = qobject_cast<PluginInterface *>(pluginLoader->instance());

    if (!interface) {
        qInfo() << "load plugin failed!!!" << pluginLoader->errorString() << pluginFile;
        pluginLoader->unload();
        pluginLoader->deleteLater();
        return;
    }

    // 保存 PluginLoader 对象指针
    if (!m_pluginAppInterList.contains(interface))
        m_pluginAppInterList.append(interface);

    QTimer::singleShot(1, this, [ = ] {
        initPlugin(interface);
    });
}

void LauncherPluginController::initPlugin(PluginInterface *interface)
{
    if (!interface)
        return;

    interface->init(this);
}

void LauncherPluginController::onSearchedTextChanged(const QString &keyword)
{
    if (keyword.isEmpty())
        return;

    QFutureWatcher<AppInfoList> *watcher = new QFutureWatcher<AppInfoList>(this);
    connect(watcher, &QFutureWatcher<AppInfoList>::finished, this, [ watcher ]() {
        const AppInfoList data = watcher->result();
        AppsManager::instance()->showSearchedData(data);
        watcher->deleteLater();
    });
    QFuture<AppInfoList> future = QtConcurrent::run([ & ]() {
            foreach (PluginInterface *inter, m_pluginAppInterList) {
                return inter->search(keyword);
            }
         return AppInfoList();
    });
    watcher->setFuture(future);
}

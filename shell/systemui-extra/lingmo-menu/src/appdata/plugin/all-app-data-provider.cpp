/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "all-app-data-provider.h"
#include "app-data-manager.h"
#include "app-folder-helper.h"
#include "user-config.h"

#include <QDebug>
#include <QDateTime>

namespace LingmoUIMenu {

AllAppDataProvider::AllAppDataProvider() : DataProviderPluginIFace()
{
    reloadFolderData();
    reloadAppData();

    connect(AppDataManager::instance(), &AppDataManager::appAdded, this, &AllAppDataProvider::onAppAdded);
    connect(AppDataManager::instance(), &AppDataManager::appDeleted, this, &AllAppDataProvider::onAppDeleted);
    connect(AppDataManager::instance(), &AppDataManager::appUpdated, this, &AllAppDataProvider::onAppUpdated);
    connect(AppFolderHelper::instance(), &AppFolderHelper::folderAdded, this, &AllAppDataProvider::onAppFolderChanged);
    connect(AppFolderHelper::instance(), &AppFolderHelper::folderDeleted, this, &AllAppDataProvider::onAppFolderChanged);
    connect(AppFolderHelper::instance(), &AppFolderHelper::folderDataChanged, this, &AllAppDataProvider::onAppFolderChanged);
}

int AllAppDataProvider::index()
{
    return 2;
}

QString AllAppDataProvider::id()
{
    return "all";
}

QString AllAppDataProvider::name()
{
    return tr("All");
}

QString AllAppDataProvider::icon()
{
    return "image://appicon/view-grid-symbolic";
}

QString AllAppDataProvider::title()
{
    return tr("All applications");
}

PluginGroup::Group AllAppDataProvider::group()
{
    return PluginGroup::SortMenuItem;
}

QVector<DataEntity> AllAppDataProvider::data()
{
    QVector<DataEntity> data;
    mergeData(data);
    return data;
}

void AllAppDataProvider::forceUpdate()
{
    reloadFolderData();
    reloadAppData();
    sendData();
}

void AllAppDataProvider::update(bool isShowed)
{
    m_windowStatus = isShowed;
    if (isShowed) {
        m_timer->blockSignals(true);
        bool isRecentDataChanged = false;
        {
            QMutexLocker locker(&m_mutex);
            for (DataEntity &appdata : m_appData) {
                bool info = appdata.isRecentInstall();
                setRecentState(appdata);
                if (appdata.isRecentInstall() != info) {
                    isRecentDataChanged = true;
                    break;
                }
            }
        }
        if (isRecentDataChanged) {
            std::sort(m_appData.begin(), m_appData.end(), appDataSort);
            sendData();
        }
    } else {
        m_timer->blockSignals(false);
        if (m_updateStatus) {
            reloadAppData();
            m_updateStatus = false;
        }
    }
}

void AllAppDataProvider::reloadAppData()
{
    QMutexLocker locker(&m_mutex);
    QVector<DataEntity> appData;

    QList<DataEntity> apps = AppDataManager::instance()->normalApps();
    if (apps.isEmpty()) {
        m_appData.swap(appData);
        return;
    }

    for (auto &app : apps) {
        if (AppFolderHelper::instance()->containApp(app.id())) {
            continue;
        }
        setRecentState(app);
        setSortPriority(app);
        appData.append(app);
    }

    std::sort(appData.begin(), appData.end(), appDataSort);
    m_appData.swap(appData);
    updateTimer();
}

void AllAppDataProvider::reloadFolderData()
{
    QMutexLocker locker(&m_mutex);
    QVector<DataEntity> folderData;

    QList<Folder> folders = AppFolderHelper::instance()->folderData();
    if (folders.isEmpty()) {
        m_folderData.swap(folderData);
        return;
    }

    DataEntity folderItem;
    for (const auto &folder : folders) {
        folderItem.setId(QString::number(folder.getId()));
        folderItem.setType(DataType::Folder);
        folderItem.setIcon(AppFolderHelper::folderIcon(folder).join(" "));
        folderItem.setName(folder.getName());

        folderData.append(folderItem);
    }

    m_folderData.swap(folderData);
}

void AllAppDataProvider::mergeData(QVector<DataEntity> &data)
{
    QMutexLocker locker(&m_mutex);
    data.append(m_folderData);
    data.append(m_appData);
}

void AllAppDataProvider::updateData(const QList<DataEntity> &apps)
{
    QMutexLocker locker(&m_mutex);
    for (const DataEntity & app : apps) {
        for (DataEntity & appdata : m_appData) {
            if (appdata.id() == app.id()) {
                appdata = app;
                setRecentState(appdata);
                setSortPriority(appdata);
                break;
            }
        }
    }
    std::sort(m_appData.begin(), m_appData.end(), appDataSort);
}

void AllAppDataProvider::updateFolderData(QStringList &idList)
{
    QList<Folder> folders = AppFolderHelper::instance()->folderData();
    if (folders.isEmpty()) {
        return;
    }
    for (const auto &folder : folders) {
        for (const auto &app : folder.getApps()) {
            if (idList.contains(app)) {
                AppFolderHelper::instance()->removeAppFromFolder(app, folder.getId());
            }
        }
    }
    AppFolderHelper::instance()->forceSync();
}

void AllAppDataProvider::updateTimer()
{
    if (m_timer == nullptr) {
        m_timer = new QTimer(this);
        m_timer->setInterval(3600000*48);
        connect(m_timer, &QTimer::timeout, this, [this]{
            if (m_windowStatus) {
                m_updateStatus = true;
            } else {
                reloadAppData();
            }
        });
    }
    m_timer->start();
}

bool AllAppDataProvider::appDataSort(const DataEntity &a, const DataEntity &b)
{
    if ((a.top() != 0) && (b.top() != 0)) {
        return a.top() < b.top();
    } else if ((a.top() == 0) && (b.top() == 0)) {
        if (a.isRecentInstall()) {
            if (b.isRecentInstall()) {
                if (QDateTime::fromString(a.insertTime(), "yyyy-MM-dd hh:mm:ss")
                        != QDateTime::fromString(b.insertTime(), "yyyy-MM-dd hh:mm:ss")) {
                    return QDateTime::fromString(a.insertTime(), "yyyy-MM-dd hh:mm:ss")
                           > QDateTime::fromString(b.insertTime(), "yyyy-MM-dd hh:mm:ss");
                } else {
                    return letterSort(a.firstLetter(), b.firstLetter());
                }
            } else {
                return true;
            }
        } else if (b.isRecentInstall()) {
            return false;
        } else {
            if (a.priority() == b.priority()) {
                return letterSort(a.firstLetter(), b.firstLetter());
            } else {
                return a.priority() > b.priority();
            }
        }
    } else {
        return a.top() > b.top();
    }
}

void AllAppDataProvider::setSortPriority(DataEntity &app)
{
    QDateTime installTime = QDateTime::fromString(app.insertTime(), "yyyy-MM-dd hh:mm:ss");
    if (installTime.isValid()) {
        qint64 appTime = installTime.secsTo(QDateTime::currentDateTime());
        if (appTime <= 3600*240) {
            appTime = appTime / (3600*24);
            double priority = app.launchTimes() * (-0.4 * (appTime^2) + 100);
            app.setPriority(priority);
            return;
        } else {
            appTime = appTime / (3600*24);
            double priority = app.launchTimes() * (240 / (appTime - 6));
            app.setPriority(priority);
            return;
        }
    }
}

void AllAppDataProvider::setRecentState(DataEntity &app)
{
    if (!UserConfig::instance()->isPreInstalledApps(app.id())) {
        if (app.launched() == 0) {
            QDateTime installTime = QDateTime::fromString(app.insertTime(), "yyyy-MM-dd hh:mm:ss");
            if (installTime.isValid()) {
                qint64 appTime = installTime.secsTo(QDateTime::currentDateTime());
                if ((appTime >= 0 ) && (appTime <= 3600*48)) {
                    app.setRecentInstall(true);
                    return;
                }
            }
        }
    }
    app.setRecentInstall(false);
}

bool AllAppDataProvider::letterSort(const QString &a, const QString &b)
{
    if (QString::compare(a, b, Qt::CaseInsensitive) == 0) {
        return false;
    }
    return QString::compare(a, b, Qt::CaseInsensitive) > 0 ? false : true;
}

void AllAppDataProvider::sendData()
{
    QVector<DataEntity> data;
    mergeData(data);
    Q_EMIT dataChanged(data);
}

void AllAppDataProvider::onAppFolderChanged()
{
    forceUpdate();
}

void AllAppDataProvider::onAppAdded(const QList<DataEntity>& apps)
{
    {
        QMutexLocker locker(&m_mutex);
        for (auto app : apps) {
            setRecentState(app);
            setSortPriority(app);
            m_appData.append(app);
        }
        std::sort(m_appData.begin(), m_appData.end(), appDataSort);
    }
    sendData();
}

void AllAppDataProvider::onAppDeleted(QStringList idList)
{
    removeApps(idList);
    reloadFolderData();
    updateFolderData(idList);
    sendData();
}

void AllAppDataProvider::removeApps(QStringList &idList)
{
    QMutexLocker locker(&m_mutex);

    QVector<DataEntity>::iterator iterator = m_appData.begin();
    while (iterator != m_appData.end() && !idList.isEmpty()) {
        if (idList.removeOne((*iterator).id())) {
            iterator = m_appData.erase(iterator);
            continue;
        }
        ++iterator;
    }
}

void AllAppDataProvider::onAppUpdated(const QList<DataEntity>& apps)
{
    updateData(apps);
    sendData();
}

} // LingmoUIMenu

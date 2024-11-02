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

#ifndef LINGMO_MENU_APP_DATA_MANAGER_H
#define LINGMO_MENU_APP_DATA_MANAGER_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include <QMap>
#include <QSet>

#include "commons.h"

namespace LingmoUIMenu {

class AppDataManager : public QObject
{
    friend class AppDataWorker;
    Q_OBJECT
public:
    static AppDataManager *instance();

    ~AppDataManager() override;

    AppDataManager(const AppDataManager& obj) = delete;
    AppDataManager &operator=(const AppDataManager& obj) = delete;
    AppDataManager(AppDataManager&& obj) = delete;
    AppDataManager &operator=(AppDataManager&& obj) = delete;

    bool getApp(const QString &appId, DataEntity &app);
    QList<DataEntity> normalApps();
    QVector<DataEntity> favoriteApps();

Q_SIGNALS:
    void appAdded(QList<DataEntity> apps);
    void appUpdated(QList<DataEntity> apps, bool totalUpdate);
    void appDeleted(QStringList idList);
    void appDataChanged();
    void favoriteAppChanged();
    void fixToFavoriteSignal(const QString &path, const int &num);
    void changedFavoriteOrderSignal(const QString &path, const int &num);
    void fixToTop(const QString &path, const int &num);
    void appLaunch(const QString &path);

private:
    AppDataManager();

private:
    QMutex m_mutex;
    QThread m_workerThread;
    QMap <QString, DataEntity> m_normalApps;
    QVector<DataEntity> m_favoriteApps;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_DATA_MANAGER_H

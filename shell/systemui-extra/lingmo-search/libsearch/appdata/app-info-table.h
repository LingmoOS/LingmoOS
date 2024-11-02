/*
 *
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef APPINFOTABLE_H
#define APPINFOTABLE_H

#include <QObject>
#include <QSqlQuery>
#include <QDir>
#include "application-property.h"

namespace LingmoUISearch {

const static QString APP_DATABASE_PATH = QDir::homePath()+"/.config/org.lingmo/lingmo-search/appdata/";
const static QString APP_DATABASE_NAME = "app-info.db";

class AppInfoTablePrivate;
class AppInfoTable : public QObject
{
    Q_OBJECT
public:
    explicit AppInfoTable(QObject *parent = nullptr);

    bool query(ApplicationPropertyMap &propertyMap, const QString &desktopFile, const ApplicationProperties& properties);
    bool query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties);
    bool query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties, const ApplicationPropertyMap& restrictions);
    bool query(ApplicationInfoMap &infoMap, const ApplicationProperties& properties, const QStringList &keywords, const ApplicationPropertyMap& restrictions);

    /**
     * @brief AppInfoTable::setAppFavoritesState
     * set the app to favorites apps(default is at 1)
     * @param desktopfp: the desktop file path of app
     */
    void setAppFavoritesState(const QString &desktopfp);

    /**
     * @brief AppInfoTable::setAppFavoritesState
     * set the favorites state of the app, you can also use to change the position of the app which is one of the Favorites Apps
     * @param desktopfp: the desktop file path of app
     * @param num: the favorites app's order(from 1)
     */
    void setAppFavoritesState(const QString &desktopfp, size_t num);

    /**
     * @brief AppInfoTable::setAppTopState
     * set the app to top apps(default is at 1)
     * @param desktopfp: desktop file path of app
     */
    void setAppTopState(const QString &desktopfp);

    /**
     * @brief AppInfoTable::setAppTopState
     * set the top state of the app, you can also use to change the position of the app which is one of the Top Apps
     * @param desktopfp: the desktop file path of app
     * @param num: the top app's order(from 1)
     */
    void setAppTopState(const QString &desktopfp, size_t num);

    void setAppLaunchedState(const QString &desktopFilePath, bool launched);


    /**
     * @brief AppInfoTable::lastError
     * the last error of the database
     * @return QString: the text of the last error
     */
    QString lastError() const;

    /**
     * @brief AppInfoTable::tranPid2DesktopFp
     * find the desktop file path of the process which corresponds to the pid
     * @param pid: the pid of the process which need to get its desktop file path
     * @param desktopfp: the desktop file path of the process corresponding to pid
     * @return bool:true if success,else false
     */
    bool tranPidToDesktopFp(uint pid, QString &desktopfp);

    bool desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath);

    bool tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopfp);

private:
    //暂不外放的接口
    bool setAppLaunchTimes(const QString &desktopfp, size_t num);
    bool setAppLockState(const QString &desktopfp, size_t num);
    bool updateAppLaunchTimes(const QString &desktopfp);
    bool searchInstallApp(QString &keyWord, QStringList &installAppInfoRes);
    bool searchInstallApp(QStringList &keyWord, QStringList &installAppInfoRes);

private:
    AppInfoTablePrivate *d;

Q_SIGNALS:
    void DBOpenFailed();
    void appDBItems2BUpdate(ApplicationInfoMap);
    void appDBItems2BUpdateAll(QStringList);
    void appDBItems2BAdd(QStringList);
    void appDBItems2BDelete(QStringList);
};
}

#endif // APPINFOTABLE_H

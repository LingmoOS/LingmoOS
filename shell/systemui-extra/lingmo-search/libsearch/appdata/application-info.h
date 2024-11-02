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
#ifndef APPLICATIONINFO_H
#define APPLICATIONINFO_H

#include <QObject>

#include "application-property.h"
namespace LingmoUISearch {
class ApplicationInfoPrivate;
class ApplicationInfo : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationInfo(QObject *parent = nullptr);
    ~ApplicationInfo();
    /**
     * @brief ApplicationInfo::getInfo 查询单个应用的单个属性
     * @param desktopFile: 要查询的应用的desktop文件路径
     * @param property: 要查询的属性
     * @return
     */
    QVariant getInfo(const QString &desktopFile, ApplicationProperty::Property property);
    /**
     * @brief ApplicationInfo::getInfo 查询单个应用的多个属性
     * @param desktopFile: 要查询的应用的desktop文件路径
     * @param properties: 要查询的属性
     * @return
     */
    ApplicationPropertyMap getInfo(const QString &desktopFile, ApplicationProperties properties);
    /**
     * @brief ApplicationInfo::getInfo 查询所有应用的多个属性
     * @param properties: 要查询的属性
     * @return
     */
    ApplicationInfoMap getInfo(ApplicationProperties properties);

    /**
     * @brief ApplicationInfo::getInfo
     * get the application info that meets the restrictions
     * @param restrictions: The restrictions that the search results should meet(e.g. u want get the app infos whose top state is 0)
     * @param properties: Each application's information should contain these properties
     * @return ApplicationInfoMap: the search result
     */
    ApplicationInfoMap getInfo(ApplicationProperties properties, ApplicationPropertyMap restrictions);

    /**
     * @brief ApplicationInfo::searchApp
     * @param properties: Each application's information should contain these properties
     * @param keyword: the keyword of this search for applications
     * @param restrictions: The restrictions that the search results should meet(e.g. u want get the app infos whose top state is 0)
     * @return ApplicationInfoMap: the search result
     */
    ApplicationInfoMap searchApp(ApplicationProperties properties, const QString &keyword, ApplicationPropertyMap restrictions);
    ApplicationInfoMap searchApp(ApplicationProperties properties, const QStringList &keywords, ApplicationPropertyMap restrictions);

    /**
     * @brief AppInfoTable::setAppToFavorites
     * set the app to favorites apps
     * @param desktopFilePath: the desktop file path of app
     */
    void setAppToFavorites(const QString &desktopFilePath);

    /**
     * @brief AppInfoTable::removeAppFromFavorites
     * remove the app from favorites apps
     * @param desktopFilePath: the desktop file path of app
     */
    void removeAppFromFavorites(const QString &desktopFilePath);

    /**
     * this function has been deprecated, use setAppToFavorites and removeAppFromFavorites instead
     * @brief AppInfoTable::setFavoritesTo
     * set the favorites state of the app to num, you can also use to change the position of the app which is one of the Favorites Apps
     * @param desktopFilePath: the desktop file path of app
     * @param num: the favorites app's position(from 1). If num is 0, it will remove the app from the favorites apps
     */
    [[deprecated]] void setFavoritesOfApp(const QString &desktopFilePath, size_t num);

    /**
     * @brief AppInfoTable::setAppToTop
     * set the app to top apps(default is at 1)
     * @param desktopFilePath: desktop file path of app
     */
    [[deprecated]] void setAppToTop(const QString &desktopFilePath);

    /**
     * @brief AppInfoTable::setAppTopTo
     * set the top state of the app to num, you can also use to change the position of the app which is one of the Top Apps
     * @param desktopFilePath: the desktop file path of app
     * @param num: the top app's position(from 1). If num is 0, it will remove the app from the top apps
     */
    [[deprecated]] void setTopOfApp(const QString &desktopFilePath, size_t num);

    void setAppLaunchedState(const QString &desktopFilePath, bool launched = true);

    /**
     * @brief ApplicationInfo::tranPid2DesktopFp
     * find the desktop file path of the process which corresponds to the pid
     * @param pid: the pid of the process which need to get its desktop file path
     * @param desktopFilePath: the desktop file path of the process corresponding to pid
     * @return bool:true if success,else false
     */
    [[deprecated]] bool tranPidToDesktopFp(int pid, QString &desktopFilePath);//obsolete
    bool tranPidToDesktopFp(uint pid, QString &desktopFilePath);

    /**
     * @brief ApplicationInfo::desktopFilePathFromName
     * find the desktop file path of the process which corresponds to the desktop file name (without .desktop)
     * @param desktopFileName: the desktop file name of the process which need to get its desktop file path
     * @param desktopFilePath: the desktop file path of the process corresponding to pid
     * @return bool:true if success,else false
     */
    bool desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath);

    /**
     * @brief ApplicationInfo::tranWinIdToDesktopFilePath
     * find the desktop file path of the process which corresponds to the winId.
     * it will find through tranPidToDesktopFp method first, and then use the winId if the desktop file can not be found by pid;
     * @param winId: the winId of the process which need to get its desktop file path(Only for X)
     * @param desktopFilePath: the desktop file path of the process corresponding to pid
     * @return bool:true if success,else false
     */
    bool tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopFilePath);

Q_SIGNALS:
    void DBOpenFailed();
    void appDBItems2BUpdate(ApplicationInfoMap);
    void appDBItems2BUpdateAll(QStringList);
    void appDBItems2BAdd(QStringList);
    void appDBItems2BDelete(QStringList);

private:
    ApplicationInfoPrivate *d = nullptr;
};

}
#endif // APPLICATIONINFO_H

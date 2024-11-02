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
 *
 */
#ifndef PENDINGAPPINFO_H
#define PENDINGAPPINFO_H

#include <QString>
#include <QStringList>
#include <QFlags>
#include "application-property.h"

namespace LingmoUISearch {

class PendingAppInfo
{
public:
    enum HandleType{
        Delete              = 0,
        Insert              = 1u,
        UpdateAll           = 1u << 1,
        UpdateLocaleData    = 1u << 2,
        UpdateLaunchTimes   = 1u << 3,
        UpdateFavorites     = 1u << 4,
        UpdateTop           = 1u << 5,
        RefreshDataBase     = 1u << 6,
        SetValue            = 1u << 7
    };
    Q_DECLARE_FLAGS(HandleTypes, HandleType)

    PendingAppInfo() = default;
    PendingAppInfo(QString desktopfp, HandleTypes type,
                   int favorites = -1, int top = -1,
                   bool addLaunch = false, int launchTimes = 0)
        : m_desktopfp(desktopfp),
          m_handleType(type),
          m_favoritesState(favorites),
          m_topState(top),
          m_willAddLaunch(addLaunch),
          m_launchTimes(launchTimes) {}

    QString path() const {return m_desktopfp;}
    QStringList pathsNeedRefreshData() const {return m_pathsNeedRefreshData;}
    HandleTypes handleType() const {return m_handleType;}
    int favoritesState() const {return m_favoritesState;}
    int topState() const {return m_topState;}
    int launchTimes() const {return m_launchTimes;}
    bool willAddLunchTimes() const {return m_willAddLaunch;}
    bool dbVersionNeedUpdate() const {return m_dbVersionNeedUpdate;}
    ApplicationInfoMap value2BSet() const {return m_appInfoMap;}

    void setDesktopFp(const QString& desktopfp) {m_desktopfp = desktopfp;}
    void setHandleType(const PendingAppInfo& info) {m_handleType = info.handleType();}
    void setHandleType(HandleTypes type) {m_handleType = type;}
    void setFavorites(int favoritesState) {m_favoritesState = favoritesState;}
    void setTop(int top) {m_topState = top;}
    void setLaunchWillAdd(bool willAdd) {m_willAddLaunch = willAdd;}
    void setLaunchTimes(int times) {m_launchTimes = times;}
    void setPathsNeedRefreshData (const QStringList& paths) {m_pathsNeedRefreshData = paths;}
    void setDBUpdate(bool versionNeedUpdate) {m_dbVersionNeedUpdate = versionNeedUpdate;}
    void setValue(const ApplicationInfoMap infoMap) {m_appInfoMap = infoMap;}
    void merge(const PendingAppInfo& info)
    {
        m_handleType |= info.handleType();

        if (info.favoritesState() != -1) {
            m_favoritesState = info.favoritesState();
        }

        if (info.topState() != -1) {
            m_topState = info.topState();
        }

        if (info.willAddLunchTimes()) {
            m_launchTimes++;
        }
    }

    bool operator ==(const PendingAppInfo& rhs) const {
        return (m_desktopfp == rhs.m_desktopfp);
    }
private:
    QString m_desktopfp;
    HandleTypes m_handleType;
    int m_favoritesState = -1;
    int m_topState = -1;
    bool m_willAddLaunch = false;
    int m_launchTimes = 0;
    QStringList m_pathsNeedRefreshData;
    bool m_dbVersionNeedUpdate = false;
    ApplicationInfoMap m_appInfoMap;
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(LingmoUISearch::PendingAppInfo::HandleTypes)

#endif // PENDINGAPPINFO_H

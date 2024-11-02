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
#ifndef APPINFOTABLEPRIVATE_H
#define APPINFOTABLEPRIVATE_H

#include "app-info-table.h"
#include <QObject>
#include <QDBusInterface>
#include <QSqlDatabase>

namespace LingmoUISearch {

class AppInfoTablePrivate : public QObject
{
    Q_OBJECT
    friend class AppInfoTable;
public:
    explicit AppInfoTablePrivate(AppInfoTable *parent = nullptr);
    AppInfoTablePrivate(AppInfoTablePrivate &) = delete;
    AppInfoTablePrivate &operator =(const AppInfoTablePrivate &) = delete;

    //设置应用的置顶和收藏
    void setAppFavoritesState(const QString &desktopfp);
    void setAppFavoritesState(const QString &desktopfp, uint num);
    void setAppTopState(const QString &desktopfp);
    void setAppTopState(const QString &desktopfp, uint num);
    void setAppLaunchedState(const QString &desktopFilePath, bool launched);

    //搜索接口
    bool searchInstallApp(QString &keyWord, QStringList &installAppInfoRes);
    bool searchInstallApp(QStringList &keyWord, QStringList &installAppInfoRes);


    //数据库错误信息
    QString lastError() const;

    //通过pid查找desktop文件
    bool tranPidToDesktopFp(uint pid, QString &desktopfp);
    bool tranWinIdToDesktopFilePath(const QVariant &winId, QString &desktopfp);
    bool desktopFilePathFromName(const QString &desktopFileName, QString &desktopFilePath);

    //下面的接口都不外放，暂时没啥用
    bool setAppLaunchTimes(const QString &desktopfp, size_t num);
    bool updateAppLaunchTimes(const QString &desktopfp);
    bool setAppLockState(const QString &desktopfp, size_t num);

private:
    ~AppInfoTablePrivate();
    bool initDateBaseConnection();
    bool openDataBase();
    void closeDataBase();

    QDBusInterface *m_signalTransInterface = nullptr;
    QDBusInterface *m_appDBInterface = nullptr;

    AppInfoTable *q = nullptr;
    QSqlDatabase *m_database = nullptr;
    QString m_ConnectionName;

public Q_SLOTS:
    void sendAppDBItemsUpdate(LingmoUISearch::ApplicationInfoMap results);
    void sendAppDBItemsUpdateAll(QStringList desktopFilePaths);
    void sendAppDBItemsAdd(QStringList desktopFilePaths);
    void sendAppDBItemsDelete(QStringList desktopFilePaths);

};


}
#endif // APPINFOTABLEPRIVATE_H

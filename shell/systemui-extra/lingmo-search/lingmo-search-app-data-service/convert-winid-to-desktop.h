/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef CONVERTDESKTOPTOWINID_H
#define CONVERTDESKTOPTOWINID_H

#include <QObject>
#include <KWindowSystem>
#include <QDir>

#define DESKTOP_FILE_PATH       "/usr/share/applications/"
#define USR_SHARE_APP_CURRENT   "/usr/share/applications/."
#define USR_SHARE_APP_UPER      "/usr/share/applications/.."
#define PEONY_TRASH             "/usr/share/applications/explor-trash.desktop"
#define PEONY_COMUTER           "/usr/share/applications/explor-computer.desktop"
#define PEONY_HOME              "/usr/share/applications/explor-home.desktop"
#define PEONY_MAIN              "/usr/share/applications/explor.desktop"

#define ANDROID_FILE_PATH       "/.local/share/applications/"
#define ANDROID_APP_CURRENT     "/.local/share/applications/."
#define ANDROID_APP_UPER        "/.local/share/applications/.."


/**
 * @brief The ConvertWinidToDesktop class
 * 需要实现的功能，desktop文件与windowId的转换
 * 传入(int)WindowId，转化为desktop文件的路径
 */

class ConvertWinidToDesktop : public QObject
{
    Q_OBJECT
public:
    static ConvertWinidToDesktop &getConverter();

    QString tranIdToDesktop(WId id);

private:
    explicit ConvertWinidToDesktop(QObject *parent = nullptr);
    ConvertWinidToDesktop(const ConvertWinidToDesktop&) = delete;
    ConvertWinidToDesktop& operator =(const ConvertWinidToDesktop&) = delete;
    ~ConvertWinidToDesktop();

    QString m_classClass = QString();
    QString m_className = QString();
    QString m_statusName = QString();
    QString m_cmdLine = QString();

    QString confirmDesktopFile(KWindowInfo info);
    QString searchFromEnviron(KWindowInfo info, QFileInfoList list);
    QString searchAndroidApp(KWindowInfo info);
    QString compareClassName(QFileInfoList list);
    QString compareCmdExec(QFileInfoList list);
    QString compareLastStrategy(QFileInfoList list);
    QString compareCmdName(QFileInfoList list);
    QString compareDesktopClass(QFileInfoList list);
    QString containsName(QFileInfoList list);
    QString getDesktopFileName(QString cmd);
};

#endif // CONVERTDESKTOPTOWINID_H

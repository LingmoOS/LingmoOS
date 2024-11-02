/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DESKTOP_FILEINFO_H__
#define __DESKTOP_FILEINFO_H__

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QSettings>

typedef struct _DTFileInfo_s {
    QString strExec = "";
    QString strStartupWMClass = "";
    QString strSimpleExec = "";
    QStringList strExecParam;
    QString strName = "";
    QString strGenericName = "";
    QString strComment = "";
    QString strIcon = "";
}DTFileInfo;

class DesktopFileInfo : public QObject
{
    Q_OBJECT
public: 
    DesktopFileInfo(QObject *parent = nullptr);
    ~DesktopFileInfo();

    static DesktopFileInfo* instance();

    QString getDesktopFileNameByExec(QString strExec);
    QString getNameByExec(QString strExec);
    QString getIconByExec(QString strExec);
    QString getNameByDesktopFile(QString strDesktopFileName);
    QString getIconByDesktopFile(QString strDesktopFileName);
    QString getAndroidAppNameByCmd(QString strCmd);
    QString getAndroidAppIconByCmd(QString strCmd);


private:
    void readAllDesktopFileInfo();
    void readCustomProcMap();
    QString exchangeCustomProcMap(QString strExec);

private:
    QMap<QString, DTFileInfo> m_mapDesktopInfoList;
    QMap<QString, QString> m_mapCustomProc;
    QSettings *m_proSettings;
    QMap<QString, DTFileInfo> m_mapAndroidAppList;
};

#endif

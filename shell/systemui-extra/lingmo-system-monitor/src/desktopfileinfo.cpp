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

#include "desktopfileinfo.h"

#include <QDirIterator>
#include <QSettings>
#include <QTextCodec>
#include <QDebug>
#include "macro.h"

Q_GLOBAL_STATIC(DesktopFileInfo, theInstance)
DesktopFileInfo *DesktopFileInfo::instance()
{
    return theInstance();
}

DesktopFileInfo::DesktopFileInfo(QObject *parent)
    : QObject(parent)
{
    m_proSettings = new QSettings(LINGMO_SYSTEM_MONITOR_CONF, QSettings::IniFormat, this);
    m_proSettings->setIniCodec("UTF-8");
    readCustomProcMap();
    readAllDesktopFileInfo();
}
    
DesktopFileInfo::~DesktopFileInfo()
{
    m_mapDesktopInfoList.clear();
}

QString DesktopFileInfo::getDesktopFileNameByExec(QString strExec)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.begin();
    strExec = exchangeCustomProcMap(strExec);
    QStringList execParamList = strExec.split(QRegExp("\\s+"));
    for (; itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec
            || itDesktopFileInfo.value().strExec == execParamList[0] || itDesktopFileInfo.value().strSimpleExec == execParamList[0]
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == strExec.toLower()
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == execParamList[0].toLower()) {
            if (itDesktopFileInfo.value().strExecParam.size() <= 1 && execParamList.size() <= 1) {
                return itDesktopFileInfo.key();
            } else if (itDesktopFileInfo.value().strExecParam.size() > 1 
                && execParamList.size() <= itDesktopFileInfo.value().strExecParam.size()) { // 实际进程命令参数长度要比预定的相同或更短
                bool isSameProc = true;
                for (int n = 1; n < itDesktopFileInfo.value().strExecParam.size(); n++) {
                    if (!itDesktopFileInfo.value().strExecParam[n].isEmpty()) {
                        if (itDesktopFileInfo.value().strExecParam[n].at(0) != '%') {   // 必传参数
                            if (execParamList.size() <= n) { // 缺少必传参数，进程不匹配
                                isSameProc = false;
                                break;
                            } else if (execParamList[n] != itDesktopFileInfo.value().strExecParam[n]) {  // 参数不必配，进程不匹配
                                isSameProc = false;
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (!isSameProc) {  // 实际cmd字段数少于desktop中exec的，按首字段匹配
                    if (itDesktopFileInfo.value().strExecParam[0] == execParamList[0]) {
                        isSameProc = true;
                    }
                }
                if (isSameProc) {
                    return itDesktopFileInfo.key();
                }
            }
        }
    }
    // 实际cmd字段数少于desktop中exec的，按首字段匹配
    for (itDesktopFileInfo = m_mapDesktopInfoList.begin(); itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec
            || itDesktopFileInfo.value().strExec == execParamList[0] || itDesktopFileInfo.value().strSimpleExec == execParamList[0]
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == strExec.toLower()
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == execParamList[0].toLower()) {
            return itDesktopFileInfo.key();
        }
    }
    return "";
}

QString DesktopFileInfo::getNameByExec(QString strExec)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.begin();
    strExec = exchangeCustomProcMap(strExec);
    QStringList execParamList = strExec.split(QRegExp("\\s+"));
    for (; itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec
            || itDesktopFileInfo.value().strExec == execParamList[0] || itDesktopFileInfo.value().strSimpleExec == execParamList[0]
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == strExec.toLower()
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == execParamList[0].toLower()) {
            if (itDesktopFileInfo.value().strExecParam.size() <= 1 && execParamList.size() <= 1) {
                if (itDesktopFileInfo.value().strName.isEmpty()) {
                    if (itDesktopFileInfo.value().strGenericName.isEmpty()) {
                        if (itDesktopFileInfo.value().strComment.isEmpty()) {
                            return "";
                        } else {
                            return itDesktopFileInfo.value().strComment;
                        }
                    } else {
                        return itDesktopFileInfo.value().strGenericName;
                    }
                } else {
                    return itDesktopFileInfo.value().strName;
                }
            } else if (itDesktopFileInfo.value().strExecParam.size() > 1 
                && execParamList.size() <= itDesktopFileInfo.value().strExecParam.size()) { // 实际进程命令参数长度要比预定的相同或更短
                bool isSameProc = true;
                for (int n = 1; n < itDesktopFileInfo.value().strExecParam.size(); n++) {
                    if (!itDesktopFileInfo.value().strExecParam[n].isEmpty()) {
                        if (itDesktopFileInfo.value().strExecParam[n].at(0) != '%') {   // 必传参数
                            if (execParamList.size() <= n) { // 缺少必传参数，进程不匹配
                                isSameProc = false;
                                break;
                            } else if (execParamList[n] != itDesktopFileInfo.value().strExecParam[n]) {  // 参数不匹配，进程不匹配
                                isSameProc = false;
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (isSameProc) {
                    if (itDesktopFileInfo.value().strName.isEmpty()) {
                        if (itDesktopFileInfo.value().strGenericName.isEmpty()) {
                            if (itDesktopFileInfo.value().strComment.isEmpty()) {
                                return "";
                            } else {
                                return itDesktopFileInfo.value().strComment;
                            }
                        } else {
                            return itDesktopFileInfo.value().strGenericName;
                        }
                    } else {
                        return itDesktopFileInfo.value().strName;
                    }
                }
            }
        }
    }
    // 实际cmd字段数少于desktop中exec的，按首字段匹配
    for (itDesktopFileInfo = m_mapDesktopInfoList.begin(); itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec
            || itDesktopFileInfo.value().strExec == execParamList[0] || itDesktopFileInfo.value().strSimpleExec == execParamList[0]
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == strExec.toLower()
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == execParamList[0].toLower()) {
            if (itDesktopFileInfo.value().strName.isEmpty()) {
                if (itDesktopFileInfo.value().strGenericName.isEmpty()) {
                    if (itDesktopFileInfo.value().strComment.isEmpty()) {
                        return "";
                    } else {
                        return itDesktopFileInfo.value().strComment;
                    }
                } else {
                    return itDesktopFileInfo.value().strGenericName;
                }
            } else {
                return itDesktopFileInfo.value().strName;
            }
        }
    }
    return "";
}

QString DesktopFileInfo::getIconByExec(QString strExec)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.begin();
    strExec = exchangeCustomProcMap(strExec);
    QStringList execParamList = strExec.split(QRegExp("\\s+"));
    for (; itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec
            || itDesktopFileInfo.value().strExec == execParamList[0] || itDesktopFileInfo.value().strSimpleExec == execParamList[0]
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == strExec.toLower()
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == execParamList[0].toLower()) {
            if (itDesktopFileInfo.value().strExecParam.size() <= 1 && execParamList.size() <= 1) {
                return itDesktopFileInfo.value().strIcon;
            } else if (itDesktopFileInfo.value().strExecParam.size() > 1 
                && execParamList.size() <= itDesktopFileInfo.value().strExecParam.size()) { // 实际进程命令参数长度要比预定的相同或更短
                bool isSameProc = true;
                for (int n = 1; n < itDesktopFileInfo.value().strExecParam.size(); n++) {
                    if (!itDesktopFileInfo.value().strExecParam[n].isEmpty()) {
                        if (itDesktopFileInfo.value().strExecParam[n].at(0) != '%') {   // 必传参数
                            if (execParamList.size() <= n) { // 缺少必传参数，进程不匹配
                                isSameProc = false;
                                break;
                            } else if (execParamList[n] != itDesktopFileInfo.value().strExecParam[n]) {  // 参数不必配，进程不匹配
                                isSameProc = false;
                                break;
                            }
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (isSameProc) {
                    return itDesktopFileInfo.value().strIcon;
                }
            }
        }
    }
    // 实际cmd字段数少于desktop中exec的，按首字段匹配
    for (itDesktopFileInfo = m_mapDesktopInfoList.begin(); itDesktopFileInfo != m_mapDesktopInfoList.end(); itDesktopFileInfo ++) {
        if (itDesktopFileInfo.value().strExec == strExec || itDesktopFileInfo.value().strSimpleExec == strExec
            || itDesktopFileInfo.value().strExec == execParamList[0] || itDesktopFileInfo.value().strSimpleExec == execParamList[0]
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == strExec.toLower()
            || itDesktopFileInfo.value().strStartupWMClass.toLower() == execParamList[0].toLower()) {
            return itDesktopFileInfo.value().strIcon;
        }
    }
    return "";
}

QString DesktopFileInfo::getNameByDesktopFile(QString strDesktopFileName)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.find(strDesktopFileName);
    if (itDesktopFileInfo != m_mapDesktopInfoList.end()) {
        if (itDesktopFileInfo.value().strName.isEmpty()) {
            if (itDesktopFileInfo.value().strGenericName.isEmpty()) {
                if (itDesktopFileInfo.value().strComment.isEmpty()) {
                    return "";
                } else {
                    return itDesktopFileInfo.value().strComment;
                }
            } else {
                return itDesktopFileInfo.value().strGenericName;
            }
        } else {
            return itDesktopFileInfo.value().strName;
        }
    }
    return "";
}

QString DesktopFileInfo::getIconByDesktopFile(QString strDesktopFileName)
{
    QMap<QString, DTFileInfo>::iterator itDesktopFileInfo = m_mapDesktopInfoList.find(strDesktopFileName);
    if (itDesktopFileInfo != m_mapDesktopInfoList.end()) {
        return itDesktopFileInfo.value().strIcon;
    }
    return "";
}

void DesktopFileInfo::readAllDesktopFileInfo()
{
    m_mapDesktopInfoList.clear();
    // read all autostart desktop file
    QDirIterator dirXdgAutostart("/etc/xdg/autostart", QDirIterator::Subdirectories);
    while(dirXdgAutostart.hasNext()) {
        dirXdgAutostart.next();
        if (dirXdgAutostart.fileInfo().suffix() == "desktop") {
            QSettings* desktopFile = new QSettings(dirXdgAutostart.filePath(), QSettings::IniFormat);
            if (desktopFile) {
                DTFileInfo dtFileInfo;
                desktopFile->setIniCodec("utf-8");
                dtFileInfo.strExec = desktopFile->value(QString("Desktop Entry/Exec")).toString();
                dtFileInfo.strExecParam = dtFileInfo.strExec.split(QRegExp("\\s+"));
                if (dtFileInfo.strExecParam.size() > 0) {
                    QStringList simpleExecList = dtFileInfo.strExecParam[0].split("/");
                    if (simpleExecList.size() > 0) {
                        dtFileInfo.strSimpleExec = simpleExecList[simpleExecList.size()-1];
                    }
                }
                dtFileInfo.strName = desktopFile->value(QString("Desktop Entry/Name[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strGenericName = desktopFile->value(QString("Desktop Entry/GenericName[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strComment = desktopFile->value(QString("Desktop Entry/Comment[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strIcon = desktopFile->value(QString("Desktop Entry/Icon")).toString();
                dtFileInfo.strStartupWMClass = desktopFile->value(QString("Desktop Entry/StartupWMClass")).toString();
                m_mapDesktopInfoList[dirXdgAutostart.fileName()] = dtFileInfo;
                //qDebug()<<"name:"<<dtFileInfo.strName<<"|"<<dtFileInfo.strGenericName<<"|"<<dtFileInfo.strComment;
                delete desktopFile;
                desktopFile = nullptr;
            }
        }
    }
    // read all application desktop file
    QDirIterator dirApplication("/usr/share/applications", QDirIterator::Subdirectories);
    while(dirApplication.hasNext()) {
        dirApplication.next();
        if (dirApplication.fileInfo().suffix() == "desktop") {
            QSettings* desktopFile = new QSettings(dirApplication.filePath(), QSettings::IniFormat);
            if (desktopFile) {
                DTFileInfo dtFileInfo;
                desktopFile->setIniCodec("utf-8");
                dtFileInfo.strExec = desktopFile->value(QString("Desktop Entry/Exec")).toString();
                dtFileInfo.strExecParam = dtFileInfo.strExec.split(QRegExp("\\s+"));
                if (dtFileInfo.strExecParam.size() > 0) {
                    QStringList simpleExecList = dtFileInfo.strExecParam[0].split("/");
                    if (simpleExecList.size() > 0) {
                        dtFileInfo.strSimpleExec = simpleExecList[simpleExecList.size()-1];
                    }
                }
                dtFileInfo.strName = desktopFile->value(QString("Desktop Entry/Name[%1]").arg(QLocale::system().name())).toString();
                if (dtFileInfo.strName.isEmpty()) {
                    dtFileInfo.strName = desktopFile->value(QString("Desktop Entry/Name")).toString();
                }
                dtFileInfo.strGenericName = desktopFile->value(QString("Desktop Entry/GenericName[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strComment = desktopFile->value(QString("Desktop Entry/Comment[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strIcon = desktopFile->value(QString("Desktop Entry/Icon")).toString();
                dtFileInfo.strStartupWMClass = desktopFile->value(QString("Desktop Entry/StartupWMClass")).toString();
                if (m_mapDesktopInfoList.find(dirApplication.fileName()) == m_mapDesktopInfoList.end())
                    m_mapDesktopInfoList[dirApplication.fileName()] = dtFileInfo;
                delete desktopFile;
                desktopFile = nullptr;
            }
        }
    }
    // read all android application desktop file
    m_mapAndroidAppList.clear();
    QDirIterator dirAndroidApp(QDir::homePath()+"/.local/share/applications", QDirIterator::Subdirectories);
    while(dirAndroidApp.hasNext()) {
        dirAndroidApp.next();
        if (dirAndroidApp.fileInfo().suffix() == "desktop") {
            QSettings* desktopFile = new QSettings(dirAndroidApp.filePath(), QSettings::IniFormat);
            if (desktopFile) {
                DTFileInfo dtFileInfo;
                desktopFile->setIniCodec("utf-8");
                dtFileInfo.strExec = desktopFile->value(QString("Desktop Entry/Exec")).toString();
                dtFileInfo.strExecParam = dtFileInfo.strExec.split(QRegExp("\\s+"));
                if (dtFileInfo.strExecParam.size() > 0) {
                    QStringList simpleExecList = dtFileInfo.strExecParam[0].split("/");
                    if (simpleExecList.size() > 0) {
                        dtFileInfo.strSimpleExec = simpleExecList[simpleExecList.size()-1];
                    }
                }
                dtFileInfo.strName = desktopFile->value(QString("Desktop Entry/Name[%1]").arg(QLocale::system().name())).toString();
                if (dtFileInfo.strName.isEmpty()) {
                    dtFileInfo.strName = desktopFile->value(QString("Desktop Entry/Name")).toString();
                }
                dtFileInfo.strGenericName = desktopFile->value(QString("Desktop Entry/GenericName[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strComment = desktopFile->value(QString("Desktop Entry/Comment[%1]").arg(QLocale::system().name())).toString();
                dtFileInfo.strIcon = desktopFile->value(QString("Desktop Entry/Icon")).toString();
                dtFileInfo.strStartupWMClass = desktopFile->value(QString("Desktop Entry/StartupWMClass")).toString();
                if (m_mapAndroidAppList.find(dirAndroidApp.fileName()) == m_mapAndroidAppList.end())
                    m_mapAndroidAppList[dirAndroidApp.fileName()] = dtFileInfo;
                delete desktopFile;
                desktopFile = nullptr;
                //qInfo()<<"AndroidApp:"<<dtFileInfo.strExec<<","<<dtFileInfo.strExecParam<<","<<dtFileInfo.strName<<","<<dtFileInfo.strIcon<<","<<dirAndroidApp.fileName();
            }
        }
    }
}

void DesktopFileInfo::readCustomProcMap()
{
    m_mapCustomProc.clear();
    m_proSettings->beginGroup("CUSTOMPROC");
    QStringList listKeys = m_proSettings->allKeys();
    for (int n = 0; n < listKeys.size(); n++) {
        m_mapCustomProc[listKeys[n]] = m_proSettings->value(listKeys[n]).toString();
    }
    m_proSettings->endGroup();
}

QString DesktopFileInfo::exchangeCustomProcMap(QString strExec)
{
    QMap<QString, QString>::iterator itCustomProc = m_mapCustomProc.begin();
    for (; itCustomProc != m_mapCustomProc.end(); itCustomProc++) {
        if (strExec.contains(itCustomProc.key(), Qt::CaseInsensitive)) {
            strExec.replace(QRegExp(itCustomProc.key(), Qt::CaseInsensitive), itCustomProc.value());
            break;
        }
    }
    return strExec;
}

QString DesktopFileInfo::getAndroidAppNameByCmd(QString strExec)
{
    strExec.remove(QRegExp("\\s+")); // 去除所有空格
    QMap<QString, DTFileInfo>::iterator itApp = m_mapAndroidAppList.begin();
    for (; itApp != m_mapAndroidAppList.end(); itApp ++) {
        if (itApp.value().strExecParam.contains(strExec)) {
            if (itApp.value().strName.isEmpty()) {
                if (itApp.value().strGenericName.isEmpty()) {
                    if (itApp.value().strComment.isEmpty()) {
                        return "";
                    } else {
                        return itApp.value().strComment;
                    }
                } else {
                    return itApp.value().strGenericName;
                }
            } else {
                return itApp.value().strName;
            }
        }
    }
    return "";
}

QString DesktopFileInfo::getAndroidAppIconByCmd(QString strExec)
{
    strExec.remove(QRegExp("\\s+")); // 去除所有空格
    QMap<QString, DTFileInfo>::iterator itApp = m_mapAndroidAppList.begin();
    for (; itApp != m_mapAndroidAppList.end(); itApp ++) {
        if (itApp.value().strExecParam.contains(strExec)) {
            return itApp.value().strIcon;
        }
    }
    return "";
}

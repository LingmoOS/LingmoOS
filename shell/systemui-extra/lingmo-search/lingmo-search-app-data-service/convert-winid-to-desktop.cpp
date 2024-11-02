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

#include <KWindowSystem>
#include <QFile>
#include <QDebug>
#include <QDir>
#include "convert-winid-to-desktop.h"

static const char* GET_DESKTOP_EXEC_NAME_MAIN = "cat %s | awk '{if($1~\"Exec=\")if($2~\"\%\"){print $1} else print}' | cut -d '=' -f 2";

ConvertWinidToDesktop::ConvertWinidToDesktop(QObject *parent) : QObject(parent)
{
}

ConvertWinidToDesktop &ConvertWinidToDesktop::getConverter()
{
    static ConvertWinidToDesktop instance;
    return instance;
}

QString ConvertWinidToDesktop::tranIdToDesktop(WId id)
{
    KWindowInfo info(id, 0, NET::WM2AllProperties);
    QString desktopName = confirmDesktopFile(info);
    qDebug() <<  "PID:" << info.pid() << "open desktopName:" << desktopName;
    return desktopName;
}

QString ConvertWinidToDesktop::confirmDesktopFile(KWindowInfo info)
{
    QString desktopFilePath = QString();
    QDir dir = QDir(DESKTOP_FILE_PATH);
    QFileInfoList list = dir.entryInfoList();
    //跳过 ./ 和 ../ 目录
    list.removeAll(QFile(USR_SHARE_APP_CURRENT));
    list.removeAll(QFile(USR_SHARE_APP_UPER));

    //第一种方法：获取点击应用时大部分desktop文件名
    desktopFilePath = searchFromEnviron(info, list);

    //第二种方法：比较名字一致性
    if (desktopFilePath.isEmpty()) {
        m_classClass = info.windowClassClass().toLower();
        m_className = info.windowClassName();

        //匹配安卓兼容
        if (m_className == "lingmo-kmre-window") {
            return searchAndroidApp(info);
        }

        QFile file(QString("/proc/%1/status").arg(info.pid()));
        if (file.open(QIODevice::ReadOnly)) {
            char buf[1024];
            qint64 len=file.readLine(buf,sizeof(buf));
            if (len!=-1) {
                m_statusName = QString::fromLocal8Bit(buf).remove("Name:").remove("\t").remove("\n");
            }
        }
        desktopFilePath = compareClassName(list);
    }

    //第三种方法：比较cmd命令行操作一致性
    if (desktopFilePath.isEmpty()) {
        QFile file(QString("/proc/%1/cmdline").arg(info.pid()));
        if (file.open(QIODevice::ReadOnly)) {
            char buf[1024];
            qint64 len=file.readLine(buf,sizeof(buf));
            if (len!=-1) {
                m_cmdLine = QString::fromLocal8Bit(buf).remove("\n");
            }
        }
        desktopFilePath = compareCmdExec(list);
    }

    //第四种方法：匹配部分字段
    if (desktopFilePath.isEmpty()) {
        desktopFilePath = compareLastStrategy(list);
    }
    return desktopFilePath;
}

QString ConvertWinidToDesktop::searchAndroidApp(KWindowInfo info)
{
    QDir androidDir = QString(QDir::homePath() + ANDROID_FILE_PATH);
    QFileInfoList androidList = androidDir.entryInfoList();
    androidList.removeAll(QDir::homePath() + ANDROID_APP_CURRENT);
    androidList.removeAll(QDir::homePath() + ANDROID_APP_UPER);

    QFile file(QString("/proc/%1/cmdline").arg(info.pid()));
    file.open(QIODevice::ReadOnly);
    QByteArray cmd = file.readAll();
    file.close();
    QList<QByteArray> cmdList = cmd.split('\0');
    for (int i = 0; i < androidList.size(); i++) {
        QFileInfo fileInfo = androidList.at(i);
        QString desktopName = fileInfo.filePath();
        if (!fileInfo.filePath().endsWith(".desktop")) {
            continue;
        }
        desktopName = desktopName.mid(desktopName.lastIndexOf("/") + 1);
        desktopName = desktopName.left(desktopName.lastIndexOf("."));
        if(desktopName == cmdList.at(10)){
            return fileInfo.filePath();
        }
    }
    return QString();
}

QString ConvertWinidToDesktop::searchFromEnviron(KWindowInfo info, QFileInfoList list)
{
    QFile file("/proc/" + QString::number(info.pid()) + "/environ");
    file.open(QIODevice::ReadOnly);
    QByteArray BA = file.readAll();
    file.close();
    QList<QByteArray> list_BA = BA.split('\0');

    QString desktopFilePath = QString();
    for (int i = 0; i < list_BA.length(); i++) {
        if (list_BA.at(i).startsWith("GIO_LAUNCHED_DESKTOP_FILE=")) {
            desktopFilePath = list_BA.at(i);
            desktopFilePath = desktopFilePath.mid(desktopFilePath.indexOf("=") + 1);
            //desktop文件地址需要重写
            desktopFilePath = desktopFilePath.mid(desktopFilePath.lastIndexOf("/") + 1);
            break;
        }
    }
    //desktop文件地址重写
    if (!desktopFilePath.isEmpty()) {
        for (int i = 0; i < list.size(); i++) {
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.filePath() == DESKTOP_FILE_PATH + desktopFilePath) {
                desktopFilePath = fileInfo.filePath();
                return desktopFilePath;
            }
        }
    }
    return desktopFilePath;
}

QString ConvertWinidToDesktop::compareClassName(QFileInfoList list)
{
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);;
        QString pathDesktopName = fileInfo.filePath();
        if (!fileInfo.filePath().endsWith(".desktop")) {
            continue;
        }
        pathDesktopName = pathDesktopName.mid(pathDesktopName.lastIndexOf("/") + 1);
        pathDesktopName = pathDesktopName.left(pathDesktopName.lastIndexOf("."));
        if (pathDesktopName == m_classClass || pathDesktopName == m_className || pathDesktopName == m_statusName)  {
            return fileInfo.filePath();
        }
    }
    return QString();
}

QString ConvertWinidToDesktop::compareCmdExec(QFileInfoList list)
{
    for (int i = 0; i < list.size(); i++) {
        QString cmd;
        QFileInfo fileInfo = list.at(i);
        if (!fileInfo.filePath().endsWith(".desktop")) {
            continue;
        }
        cmd.asprintf(GET_DESKTOP_EXEC_NAME_MAIN, fileInfo.filePath().toStdString().data());
        QString desktopFileExeName = getDesktopFileName(cmd).remove("\n");

        if (desktopFileExeName.isEmpty()) {
            continue;
        }

        if (desktopFileExeName == m_cmdLine || desktopFileExeName.startsWith(m_cmdLine) || m_cmdLine.startsWith(desktopFileExeName)) {
            return fileInfo.filePath();
        }

        //仅仅是为了适配微信
        desktopFileExeName = "/usr/lib/" + desktopFileExeName;
        if (desktopFileExeName == m_cmdLine || desktopFileExeName.startsWith(m_cmdLine) || m_cmdLine.startsWith(desktopFileExeName)) {
            return fileInfo.filePath();
        }
    }
    return QString();
}

//最后的匹配策略汇总
QString ConvertWinidToDesktop::compareLastStrategy(QFileInfoList list)
{
    QString desktopFilePath = compareCmdName(list);

    if (desktopFilePath.isEmpty()) {
        desktopFilePath = compareDesktopClass(list);
    }

    if (desktopFilePath.isEmpty()) {
        desktopFilePath = containsName(list);
    }
    return desktopFilePath;
}

QString ConvertWinidToDesktop::compareCmdName(QFileInfoList list)
{
    for (int i = 0; i < list.size(); i++) {
        QString cmd;
        QFileInfo fileInfo = list.at(i);
        if (!fileInfo.filePath().endsWith(".desktop")) {
            continue;
        }
        cmd.asprintf(GET_DESKTOP_EXEC_NAME_MAIN, fileInfo.filePath().toStdString().data());
        QString desktopFileExeName = getDesktopFileName(cmd).remove("\n");

        if (desktopFileExeName.isEmpty()) {
            continue;
        }

        if (desktopFileExeName.startsWith(m_className) || desktopFileExeName.endsWith(m_className)) {
            return fileInfo.filePath();
        }
    }
    return QString();
}

QString ConvertWinidToDesktop::compareDesktopClass(QFileInfoList list)
{
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QString pathDesktopName = fileInfo.filePath();
        if (!fileInfo.filePath().endsWith(".desktop")) {
            continue;
        }
        pathDesktopName = pathDesktopName.mid(pathDesktopName.lastIndexOf("/") + 1);
        pathDesktopName = pathDesktopName.left(pathDesktopName.lastIndexOf("."));

        if (pathDesktopName.startsWith(m_className) || pathDesktopName.endsWith(m_className)) {
            return fileInfo.filePath();
        }
        else if (m_className.startsWith(pathDesktopName) || m_className.endsWith(pathDesktopName)) {
            return fileInfo.filePath();
        }
    }
    return QString();
}

QString ConvertWinidToDesktop::containsName(QFileInfoList list)
{
    for (int i = 0; i < list.size(); i++) {
        QString cmd;
        QFileInfo fileInfo = list.at(i);
        QString pathDesktopName = fileInfo.filePath();

        if (!fileInfo.filePath().endsWith(".desktop")) {
            continue;
        }

        cmd.asprintf(GET_DESKTOP_EXEC_NAME_MAIN, fileInfo.filePath().toStdString().data());
        QString desktopFileExeName = getDesktopFileName(cmd).remove("\n");

        pathDesktopName = pathDesktopName.mid(pathDesktopName.lastIndexOf("/") + 1);
        pathDesktopName = pathDesktopName.left(pathDesktopName.lastIndexOf("."));

        if (pathDesktopName.contains(m_className) || desktopFileExeName.contains(m_className)) {
            return fileInfo.filePath();
        }
    }
    return QString();
}

//执行头文件中宏定义写好的终端指令获取对应的Exec字段
QString ConvertWinidToDesktop::getDesktopFileName(QString cmd)
{
    char name[200];
    FILE *fp1 = NULL;
    if ((fp1 = popen(cmd.toStdString().data(), "r")) == NULL) {
        return QString();
    }
    memset(name, 0, sizeof(name));
    fgets(name, sizeof(name), fp1);
    pclose(fp1);
    return QString(name);
}

ConvertWinidToDesktop::~ConvertWinidToDesktop()
{
}

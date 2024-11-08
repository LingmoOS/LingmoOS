/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SYSTEMDBUSREGISTER_H
#define SYSTEMDBUSREGISTER_H

#include <QtDBus/QDBusContext>
#include <QObject>
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QStorageInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>

class SystemDbusRegister : public QObject, protected QDBusContext
{
    Q_OBJECT
//    Q_CLASSINFO("D-Bus Interface", "com.lingmousbcreator.interface")
    Q_CLASSINFO("D-Bus Interface", "com.lingmousbcreator.interface")
public:
    SystemDbusRegister();
//    ~SystemDbusRegister();
    bool mountDevice(QString target);

    /**
     * @brief 判断程序是否已开启
     * @param processName
     * @return bool
     */
    static bool isRunning(const QString &processName);
Q_SIGNALS:
    void workingProgress(int);
    void errorstr(QString);
    void makeFinish(QString);
    void authorityStatus(QString);

public slots:
    Q_SCRIPTABLE void MakeStart(QString sourcePath,QString targetPath);
    Q_SCRIPTABLE void MakeExit();
    void dd_finished(int,QProcess::ExitStatus);
    void mkfs_finished(int,QProcess::ExitStatus);
    void readStandardOutput(int exitCode,QProcess::ExitStatus exitStatus);
    void syncreadStandardOutput(int exitCode,QProcess::ExitStatus exitStatus);
private:
    void readBashStandardErrorInfo();
    void handletimeout();
    void checkmd5finish();
    void finishEvent();
    bool unmountDevice(QString);//制作开始前卸载设备
    void isMakingSucess();
    bool umountPath(QString); //制作完成后卸载路径
    void getmountpoint();
    void checkmd5();
    QJsonArray QStringToJsonArray(const QString);
private:
    QProcess *sync=nullptr;
    QProcess umount;
    QProcess mount;
    QProcessEnvironment env;
    QString uDiskPath = "";
    qint64 sourceFileSize = 0;
    QProcess *command_dd = nullptr;
    QProcess *md5=nullptr;
    QProcess *bash=nullptr;
    bool m_isStarted = false;
    bool m_isFinished = false;
    QString mountpoint;      //挂载点
    QString m_userName = nullptr; //当前用户
    QTimer *time;
    QString sourceFile;
    QTimer *timer;
    QTimer *checkMd5FinishTimer = nullptr;
    bool m_isMd5Finish=false;
    bool m_getmountpoint=false;
    bool m_isdeviceexist=false;
    QString m_isFinishstatus;
    QString MD5str;
    QString mkfsoutput;
    QString mkfserror;
    QString mountstr;
    QString mountpath;   //挂载路径
    bool ismakesuccess =false;
};

#endif // SYSTEMDBUSREGISTER_H

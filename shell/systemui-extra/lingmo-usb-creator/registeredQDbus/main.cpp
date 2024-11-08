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
#include <QCoreApplication>
#include <QDebug>
#include <lingmo-log4qt.h>
#include <QDBusConnection>
#include "systemdbusregister.h"
#include <QMutex>
#include <QFile>
#include <QDateTime>
#define PROC_LOG "/var/log/usb-boot-maker-service.log"
#define END_LINE "\n"
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 加锁
    static QMutex mutex;
    mutex.lock();

    QByteArray localMsg = msg.toLocal8Bit();

    QString strMsg("");
    switch(type)
    {
    case QtDebugMsg:
        strMsg = QString("Debug:");
        break;
    case QtWarningMsg:
        strMsg = QString("Warning:");
        break;
    case QtCriticalMsg:
        strMsg = QString("Critical:");
        break;
    case QtFatalMsg:
        strMsg = QString("Fatal:");
        break;
    }

    // 设置输出信息格式
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString strMessage = QString("Message:%1 File:%2  Line:%3  Function:%4  DateTime:%5")
            .arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function).arg(strDateTime);
    static QString m_sysRootPath;
    m_sysRootPath = "/";
    QString fileName = m_sysRootPath + PROC_LOG;
    fileName.replace("//", "/");
    // 输出信息至文件中（读写、追加形式）
    QFile file(fileName);
    file.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream stream(&file);
    stream << strMessage <<END_LINE;
    file.flush();
    file.close();

    // 解锁
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    //init log module
//    initLingmoUILog4qt("usb-boot-maker-service");
    QCoreApplication a(argc, argv);
    a.setOrganizationName("Lingmo Team");
    a.setApplicationName("system-bus-lingmousbcreator-service");
    //regist service
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerService("com.lingmousbcreator.systemdbus")){
        qCritical() << "QDbus register service failed reason:" << systemBus.lastError();
        exit(1);
    }
    //regist object
    if (!systemBus.registerObject("/", new SystemDbusRegister(), QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)){
        qCritical() << "QDbus register object failed reason:" << systemBus.lastError();
        exit(2);
    }

    //regist signals

    qDebug() << "ok";

    return a.exec();
}

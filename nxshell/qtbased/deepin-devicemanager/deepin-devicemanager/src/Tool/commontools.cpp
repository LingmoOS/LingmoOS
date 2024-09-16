// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commontools.h"
#include "DDLog.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QDir>

DWIDGET_USE_NAMESPACE
using namespace DDLog;
#define ICON_SIZE_WIDTH 36
#define ICON_SIZE_HEIGHT 36

QMap<DriverType, QString> CommonTools::m_MapDriverIcon = {
    {DR_Bluetooth, QString(":/icons/deepin/builtin/icons/bluetooth.svg")}
    , {DR_Camera, QString(":/icons/deepin/builtin/icons/image.svg")}
    , {DR_Gpu, QString(":/icons/deepin/builtin/icons/GPU.svg")}
    , {DR_Keyboard, QString(":/icons/deepin/builtin/icons/keyboard.svg")}
    , {DR_Sound, QString(":/icons/deepin/builtin/icons/voice.svg")}
    , {DR_Mouse, QString(":/icons/deepin/builtin/icons/mouse.svg")}
    , {DR_Network, QString(":/icons/deepin/builtin/icons/network.svg")}
    , {DR_Printer, QString(":/icons/deepin/builtin/icons/printer.svg")}
    , {DR_Scaner, QString(":/icons/deepin/builtin/icons/scaner.svg")}
    , {DR_Tablet, QString(":/icons/deepin/builtin/icons/tablet.svg")}
    , {DR_WiFi, QString(":/icons/deepin/builtin/icons/Wi-Fi.svg")}
    , {DR_OtherDevice, QString(":/icons/deepin/builtin/icons/scaner.svg")}
};

QMap<DriverType, QString> CommonTools::m_MapDriverType = {
    {DR_Bluetooth, QObject::tr("Bluetooth adapter")}
    , {DR_Camera, QObject::tr("Imaging device")}
    , {DR_Gpu, QObject::tr("Display adapter")}
    , {DR_Keyboard, QObject::tr("Keyboard")}
    , {DR_Sound, QObject::tr("Sound card")}
    , {DR_Mouse, QObject::tr("Mouse")}
    , {DR_Network, QObject::tr("Network adapter")}
    , {DR_Printer, QObject::tr("Printer")}
    , {DR_Scaner, QObject::tr("Imaging device")}
    , {DR_Tablet, QObject::tr("Other Devices")}
    , {DR_WiFi, QObject::tr("Wireless network adapter")}
    , {DR_OtherDevice, QObject::tr("Other Devices")}
};

QMap<Status, QString> CommonTools::m_MapStatusIcon = {
    {ST_SUCESS, QString(":/icons/deepin/builtin/icons/ok.svg")}
    , {ST_FAILED, QString(":/icons/deepin/builtin/icons/warning.svg")}
    , {ST_DOWNLOADING, QString("")}
    , {ST_INSTALL, QString("")}
    , {ST_NOT_INSTALL, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_CAN_UPDATE, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_WAITING, QString(":/icons/deepin/builtin/icons/waiting.svg")}
    , {ST_DRIVER_NOT_BACKUP, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_DRIVER_BACKING_UP, QString("")}
    , {ST_DRIVER_BACKUP_FAILED, QString(":/icons/deepin/builtin/icons/warning.svg")}
    , {ST_DRIVER_BACKUP_SUCCESS, QString(":/icons/deepin/builtin/icons/ok.svg")}
    , {ST_DRIVER_RESTORING, QString("")}
};

QMap<Status, QString> CommonTools::m_MapStatusType = {
    {ST_SUCESS, QObject::tr("Installation successful")}
    , {ST_FAILED, QObject::tr("Installation failed")}
    , {ST_DOWNLOADING, QObject::tr("Downloading")}
    , {ST_INSTALL, QObject::tr("Installing")}
    , {ST_NOT_INSTALL, QObject::tr("Not installed")}
    , {ST_CAN_UPDATE, QObject::tr("Out-of-date")}
    , {ST_WAITING, QObject::tr("Waiting")}
    , {ST_DRIVER_NOT_BACKUP, QObject::tr("Not backed up")}
    , {ST_DRIVER_BACKING_UP, QObject::tr("Backing up")}
    , {ST_DRIVER_BACKUP_FAILED, QObject::tr("Backup failed")}
    , {ST_DRIVER_BACKUP_SUCCESS, QObject::tr("Backup successful")}
    , {ST_DRIVER_RESTORING, QObject::tr("Restoring")}
};

QMap<int, QString> CommonTools::m_MapErrorString = {
    {EC_NULL, QObject::tr("Unknown error")}
    , {EC_NETWORK, QObject::tr("Network error")}
    , {EC_CANCEL, QObject::tr("Canceled")}
    , {EC_NOTFOUND, QObject::tr("Failed to get driver files")}
    , {EC_NOTIFY_NETWORK, tr("EC_NOTIFY_NETWORK")}
    , {EC_REINSTALL, tr("EC_REINSTALL")}
    , {EC_6, tr("EC_6")}
};

CommonTools::CommonTools(QObject *parent)
    : QObject(parent)
{

}

QString CommonTools::getDriverType(DriverType type)
{
    return m_MapDriverType[type];
}

QString CommonTools::getDriverPixmap(DriverType type)
{
    return m_MapDriverIcon[type];
}

QString CommonTools::getStausType(Status type)
{
    return m_MapStatusType[type];
}

QString CommonTools::getStatusPixmap(Status type)
{
    return m_MapStatusIcon[type];
}

QString CommonTools::getSystemTime()
{
    QDateTime time = QDateTime::currentDateTime();
    QString date = time.toString("yyyy-M-d hh:mm");
    return date;
}

QString CommonTools::getErrorString(int err)
{
    return m_MapErrorString[err];
}

void CommonTools::feedback()
{
    QDBusInterface interface("com.deepin.dde.ServiceAndSupport",
                              "/com/deepin/dde/ServiceAndSupport",
                              "com.deepin.dde.ServiceAndSupport");

     //    selfSupport         = 0, //自助支持
     //    messageConsultation = 1, //留言咨询
     //    customerChat        = 2, //在线客服
     //    contentUs           = 3  //联系我们
     uint8_t supporttype = 1;
     QDBusReply<void> reply = interface.call("ServiceSession", supporttype);
     if (reply.isValid()) {
         qCDebug(appLog) << "call com.deepin.dde.ServiceAndSupport success";
     } else {
         qCDebug(appLog) << "call com.deepin.dde.ServiceAndSupport failed";
     }
}

QString CommonTools::getUrl()
{
    QFile file(QDir::homePath() + "/url");
    if(!file.open(QIODevice::ReadOnly)){
        return "https://driver.uniontech.com/api/v1/drive/search";
    }
    QString info = file.readAll();
    if("true" == info){
        return "https://driver.uniontech.com/api/v1/drive/search";
    }else{
        return "https://drive-pre.uniontech.com/api/v1/drive/search";
    }
}

QString CommonTools::getBackupPath()
{
    return "/usr/share/deepin-devicemanager/";
}

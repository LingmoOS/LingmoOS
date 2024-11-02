/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef CONFIG_H
#define CONFIG_H

#include <QDir>
#include <QGSettings>
#include <QString>
#include <QByteArray>
#include <QDBusInterface>
#include <QDebug>
#include <QProcess>
#include <QStyle>
#include <QPixmap>
#include <KWindowSystem>
#include <QGuiApplication>
#include <QCursor>
#include <QScreen>
#include <QPixmap>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <lingmo-log4qt.h>

#include "windowmanager/windowmanager.h"

#define INFO_PRINT(info)                    qInfo() << Q_FUNC_INFO << info << "in line :" << __LINE__
#define DEBUG_PRINT(info)                   qInfo() << Q_FUNC_INFO << info << "in line :" << __LINE__

#define PANEL_SETTINGS                      "org.lingmo.panel.settings"
#define PANEL_SIZE_KEY                      "panelsize"
#define PANEL_POSITION_KEY                  "panelposition"

#define MEDIA_SOUND_SERVICE                 "org.lingmo.sound.theme.player"
#define MEDIA_SOUND_PATH                    "/org/lingmo/sound/theme/player"
#define MEDIA_SOUND_INTERFACE               MEDIA_SOUND_SERVICE

#define MEDIA_BATTERY_SERVICE               "org.lingmo.media"
#define MEDIA_BATTERY_PATH                  "/org/lingmo/media/bluetooth"
#define MEDIA_BATTERY_INTERFACE             MEDIA_BATTERY_SERVICE

#define SYSTEMD_SERVICE                     "com.lingmo.bluetooth"
#define SYSTEMD_PATH                        "/com/lingmo/bluetooth"
#define SYSTEMD_INTERFACE                   SYSTEMD_SERVICE

#define SESSION_SERVICE                     "com.lingmo.bluetooth"
#define SESSION_PATH                        "/com/lingmo/bluetooth"
#define SESSION_INTERFACE                   SESSION_SERVICE

#define SYSTEM_ACTIVE_USER_DBUS             "org.lingmo.test"
#define SYSTEM_ACTIVE_USER_PATH             "/com/lingmo/test"
#define SYSTEM_ACTIVE_USER_INTERFACE        "com.lingmo.test"

//org.freedesktop.DBus
#define SESSION_DBUS_FREEDESKTOP_SERVICE    "org.freedesktop.DBus"
#define SESSION_DBUS_FREEDESKTOP_PATH       "/org/freedesktop/DBus"
#define SESSION_DBUS_FREEDESKTOP_INTERFACE  "org.freedesktop.DBus"

//音视频session dbus
#define SESSION_DBUS_NAME_STR_FIELD         "org.mpris.MediaPlayer2"
#define SESSION_DBUS_PLAYER_PATH            "/org/mpris/MediaPlayer2"
#define SESSION_DBUS_PLAYER_INTERFACE       "org.mpris.MediaPlayer2.Player"

#define GSETTING_SCHEMA_LINGMOBLUETOOH        "org.lingmo.bluetooth"
#define GSETTING_SCHEMA_UKCC                "org.lingmo.control-center.plugins"
#define GSETTING_PACH_UKCC                  "/org/lingmo/control-center/plugins/Bluetooth/"

#define SESSION_DBUS_CONNECT                QDBusConnection::sessionBus().connect
#define SYSTEM_DBUS_CONNECT                 QDBusConnection::systemBus().connect

#define CONNECT_DBUS_SIGNAL(signame, func) \
                                    SESSION_DBUS_CONNECT(SESSION_SERVICE, \
                                    SESSION_PATH, \
                                    SESSION_INTERFACE, \
                                    signame, \
                                    this,\
                                    SLOT(func))

#define TRANSFER_DBUS_SIGNAL(signame, func) \
                                    SESSION_DBUS_CONNECT(SESSION_SERVICE, \
                                    SESSION_PATH, \
                                    SESSION_INTERFACE, \
                                    signame, \
                                    this,\
                                    SIGNAL(func))

#define CREATE_METHOD_CALL(funcName) \
                                    QDBusMessage::createMethodCall(SESSION_SERVICE, \
                                    SESSION_PATH, \
                                    SESSION_INTERFACE, \
                                    funcName)

#define CONNECT_SYSTEM_DBUS_SIGNAL(signame, func) \
                                    SYSTEM_DBUS_CONNECT(SYSTEMD_SERVICE, \
                                    SYSTEMD_PATH, \
                                    SYSTEMD_INTERFACE, \
                                    signame, \
                                    this,\
                                    SLOT(func))

#define TRANSFER_SYSTEM_DBUS_SIGNAL(signame, func) \
                                    SYSTEM_DBUS_CONNECT(SYSTEMD_SERVICE, \
                                    SYSTEMD_PATH, \
                                    SYSTEMD_INTERFACE, \
                                    signame, \
                                    this,\
                                    SIGNAL(func))

#define CREATE_SYSTEM_METHOD_CALL(funcName) \
                                    QDBusMessage::createMethodCall(SYSTEMD_SERVICE, \
                                    SYSTEMD_PATH, \
                                    SYSTEMD_INTERFACE, \
                                    funcName)

#define CALL_METHOD(dbusmsg) QDBusConnection::sessionBus().call(dbusmsg);
#define SYSTEM_CALL_METHOD(dbusmsg) QDBusConnection::systemBus().call(dbusmsg);

enum Environment
{
    NOMAL = 0,
    HUAWEI,
    LAIKA,
    MAVIS
};
extern int envPC;

typedef enum Status {
    /** Indicates that the transfer is queued. */
    Queued,
    /** Indicates that the transfer is active. */
    Active,
    /** Indicates that the transfer is suspended. */
    Suspended,
    /** Indicates that the transfer have completed successfully. */
    Complete,
    /** Indicates that the transfer have failed with error. */
    Error,
    /** Indicates that the transfer status is unknown. */
    Unknown,
    /** Indicates that the transfer status is static. */
    Static = 0xff
} SendStatus;

enum Type {
    /** The device is a phone. */
    Phone = 0,
    /** The device is a modem. */
    Modem,
    /** The device is a computer. */
    Computer,
    /** The device is a network. */
    Network,
    /** The device is a headset. */
    Headset,
    /** The device is a headphones. */
    Headphones,
    /** The device is an uncategorized audio video device. */
    AudioVideo,
    /** The device is a keyboard. */
    Keyboard,
    /** The device is a mouse. */
    Mouse,
    /** The device is a joypad. */
    Joypad,
    /** The device is a graphics tablet (input device). */
    Tablet,
    /** The deivce is an uncategorized peripheral device. */
    Peripheral,
    /** The device is a camera. */
    Camera,
    /** The device is a printer. */
    Printer,
    /** The device is an uncategorized imaging device. */
    Imaging,
    /** The device is a wearable device. */
    Wearable,
    /** The device is a toy. */
    Toy,
    /** The device is a health device. */
    Health,
    /** The device is not of any of the known types. */
    Uncategorized
};

enum ConnectionErrMsg
{
    ERR_BREDR_CONN_SUC,
    ERR_BREDR_CONN_ALREADY_CONNECTED = 1,
    ERR_BREDR_CONN_PAGE_TIMEOUT,
    ERR_BREDR_CONN_PROFILE_UNAVAILABLE,
    ERR_BREDR_CONN_SDP_SEARCH,
    ERR_BREDR_CONN_CREATE_SOCKET,
    ERR_BREDR_CONN_INVALID_ARGUMENTS,
    ERR_BREDR_CONN_ADAPTER_NOT_POWERED,
    ERR_BREDR_CONN_NOT_SUPPORTED,
    ERR_BREDR_CONN_BAD_SOCKET,
    ERR_BREDR_CONN_MEMORY_ALLOC,
    ERR_BREDR_CONN_BUSY,
    ERR_BREDR_CONN_CNCR_CONNECT_LIMIT,
    ERR_BREDR_CONN_TIMEOUT,
    ERR_BREDR_CONN_REFUSED,
    ERR_BREDR_CONN_ABORT_BY_REMOTE,
    ERR_BREDR_CONN_ABORT_BY_LOCAL,
    ERR_BREDR_CONN_LMP_PROTO_ERROR,
    ERR_BREDR_CONN_CANCELED,
    ERR_BREDR_CONN_UNKNOWN,

 ////////////////////////////////////////////

    ERR_LE_CONN_INVALID_ARGUMENTS = 20,
    ERR_LE_CONN_ADAPTER_NOT_POWERED,
    ERR_LE_CONN_NOT_SUPPORTED,
    ERR_LE_CONN_ALREADY_CONNECTED,
    ERR_LE_CONN_BAD_SOCKET,
    ERR_LE_CONN_MEMORY_ALLOC,
    ERR_LE_CONN_BUSY,
    ERR_LE_CONN_REFUSED,
    ERR_LE_CONN_CREATE_SOCKET,
    ERR_LE_CONN_TIMEOUT,
    ERR_LE_CONN_SYNC_CONNECT_LIMIT,
    ERR_LE_CONN_ABORT_BY_REMOTE,
    ERR_LE_CONN_ABORT_BY_LOCAL,
    ERR_LE_CONN_LL_PROTO_ERROR,
    ERR_LE_CONN_GATT_BROWSE,
    ERR_LE_CONN_UNKNOWN,

////////////////////////////////////////////////////

    ERR_BREDR_Invalid_Arguments = 36,
    ERR_BREDR_Operation_Progress,
    ERR_BREDR_Already_Exists,
    ERR_BREDR_Operation_Not_Supported,
    ERR_BREDR_Already_Connected,
    ERR_BREDR_Operation_Not_Available,
    ERR_BREDR_Does_Not_Exist,
    ERR_BREDR_Does_Not_Connected,
    ERR_BREDR_Does_In_Progress,
    ERR_BREDR_Operation_Not_Authorized,
    ERR_BREDR_No_Such_Adapter,
    ERR_BREDR_Agent_Not_Available,
    ERR_BREDR_Resource_Not_Ready,


 /************上面错误码为bluez返回错误码****************/
    ERR_BREDR_Bluezqt_DidNot_ReceiveReply,


 /////////////////////////////////////////////////
    ERR_BREDR_INTERNAL_NO_Default_Adapter,
    ERR_BREDR_INTERNAL_Operation_Progress,
    ERR_BREDR_INTERNAL_Already_Connected,
    ERR_BREDR_INTERNAL_Dev_Not_Exist,
    ERR_BREDR_UNKNOWN_Other,
};

enum PanelPosition{
    Bottom = 0, //!< The bottom side of the screen.
    Top,    //!< The top side of the screen.
    Left,   //!< The left side of the screen.
    Right   //!< The right side of the screen.
};

struct struct_pos
{
    struct_pos() {}
    struct_pos(int _x, int _y, int _width, int _high){
        x = _x;
        y = _y;
        width = _width;
        high = _high;
    }
    int x = 0;
    int y = 0;
    int width = 0;
    int high = 0;
};


class Config : public QObject
{
    Q_OBJECT
public:

    enum PixmapColor {
            WHITE = 0,
            BLACK,
            GRAY,
            BLUE,
    };
    Q_ENUM(PixmapColor)

    Config(QObject *parent = nullptr,QString name = GSETTING_SCHEMA_LINGMOBLUETOOH);
    ~Config();

    static  int pairFuncReply(QString, bool);
    static  int replyFileReceiving(QString, bool, QString, QString);
    static  int activeConnectionReply(QString, bool);
    static  int devConnect(QString);
    static  int devDisconnect(QString);
    static  int devRemove(QString, QString);
    static  bool setDefaultAdapterAttr(QMap<QString, QVariant> adpAttr);

    static void soundWarning();
    static void soundComplete();
    static void SendNotifyMessage(QString title, QString, QString soundeffects);
    static void OpenBluetoothSettings();
    static void CommandToOpenBluetoothSettings();
    static void AppManagerToOpenBluetoothSettings();
    static const QPixmap loadSvg(const QPixmap &source, const PixmapColor &color);
    static void setKdkGeometry(QWindow *windowHandle, int width, int height, bool center);
    static void setKdkGeometry(QWindow *windowHandle, int width, int height,
                               int panelPosition, int panelSize, bool center);
    static struct_pos getWidgetPos(int ax, int ay, int aw, int ah, int width);
private:
    QString setting_name;
    QGSettings    *gsetting = nullptr;
    friend class MainProgram;
    friend class TrayWidget;
};
#endif // CONFIG_H

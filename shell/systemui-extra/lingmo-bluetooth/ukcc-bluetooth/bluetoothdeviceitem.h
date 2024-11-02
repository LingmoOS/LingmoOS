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

#ifndef BLUETOOTHDEVICEITEM_H
#define BLUETOOTHDEVICEITEM_H

#include "devicebase.h"
#include "loadinglabel.h"
#include "devremovedialog.h"
#include "bluetoothdevicefunc.h"
#include "bluetoothmainwindow.h"
#include "bluetoothdbusservice.h"

#include <QChar>
#include <QIcon>
#include <QMenu>
#include <QFont>
#include <QFrame>
#include <QLabel>
#include <QColor>
#include <QTimer>
#include <QDebug>
#include <QStyle>
#include <QObject>
#include <QString>
#include <QPainter>
#include <QDateTime>
#include <QPushButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFontMetrics>
#include <QApplication>
#include <QAbstractButton>
#include <QGSettings/QGSettings>

#include "windowmanager/windowmanager.h"

#define CONNECT_ERROR_COUNT         (3)
#define CONNECT_DEVICE_TIMING       (35*1000)
#define CONNECT_ERROR_TIMER_CLEAR   (3*60*1000) //3分钟内用户来连接失败超过3次，提示用户是否移除后重新配对连接

#define DEV_ICON_WH     16,16

class bluetoothdeviceitem : public QPushButton
{
    Q_OBJECT
public:

    enum _DEV_STATUS {
        Dev_Connecting,
        Dev_Disconnecting,
        Dev_NotPaired,
        Dev_NotConnected,
        Dev_ConnectionFail,
        Dev_DisconnectionFail
    };//(_DEV_STATUS);

    bluetoothdeviceitem(QString dev_address ,QWidget *parent = nullptr);
    ~bluetoothdeviceitem();
    QString m_str_unknown = tr("unknown");
    QString m_str_connecting = tr("Connecting");
    QString m_str_disconnecting = tr("Disconnecting");
    QString m_str_notpaired = tr("Not Paired");
    QString m_str_notconnected = tr("Not Connected");
    QString m_str_connected = tr("Connected");
    QString m_str_connectionfail = tr("Connect fail,Please try again");
    QString m_str_disconnectionfail = tr("Disconnection Fail");

private:

    QHBoxLayout *   devItemHLayout = nullptr;
    QHBoxLayout *   devFuncHLayout = nullptr;
    QLabel      *   devIconLabel   = nullptr;
    QLabel      *   devNameLabel   = nullptr;
    QLabel      *   devStatusLabel = nullptr;
//    QFrame      *   devFuncFrame   = nullptr;
    bluetoothdevicefunc     * devFuncBtn   = nullptr;
    LoadingLabel    * devloadingLabel =  nullptr;

    QTimer          * devConnectiontimer = nullptr;
    QTimer          * devConnectionFail_timer = nullptr;

    QString getDevName();
    QString getDevStatus();
    QPixmap getDevTypeIcon();

    void    initGsettings();
    void    initBackground();
    void    initInterface();
    void    refreshInterface();
    void    devStatusLoading();
    void    bindInInterfaceUISignals();
    void    bindDeviceChangedSignals();
    void    refreshDevCurrentStatus();
    void    devConnectionFail();
    void    showDeviceRemoveWidget(DevRemoveDialog::REMOVE_INTERFACE_TYPE);


    uint        _mConnFailCount = 0;
    bool        _themeIsBlack = false;
    QString     _mIconThemeName;
    long long   _pressCurrentTime;
    QString     _MDev_addr ;
    QGSettings * _mStyle_GSettings  = nullptr;

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *e);

signals:

    void devFuncOptSignals();
    void devPairedSuccess(QString);
    void devConnectedChanged(bool);
    void devRssiChanged(qint64);
    void bluetoothDeviceItemRemove(QString);

private Q_SLOTS:
    void devFuncOperationSlot();
    void devConnOperationTimeoutSlot();
    void devItemNameChanged(const QString &);
    void devItemTypeChanged(const bluetoothdevice::DEVICE_TYPE &);
    void devItemStatusChanged(const QString &);
    void mStyle_GSettingsSlot(const QString &key);
    void devBtnPressedSlot();
    void devFuncOpertionRemoveSlot(QString);

};

#endif // BLUETOOTHDEVICEITEM_H

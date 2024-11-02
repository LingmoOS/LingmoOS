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

#ifndef MAINPROGRAM_H
#define MAINPROGRAM_H

#include "dbus/bluetoothdbus.h"
#include "config/config.h"
#include "component/kyfiledialog.h"
#include "popwidget/traywidget.h"
#include "popwidget/pincodewidget.h"
#include "popwidget/errormessagewidget.h"
#include "popwidget/activeconnectionwidget.h"
#include "popwidget/filereceivingpopupwidget.h"
#include "popwidget/bluetoothfiletransferwidget.h"
#include "lingmostylehelper/lingmostylehelper.h"

#include <glib.h>
#include <glib/gprintf.h>

#include <QString>
#include <QDebug>
#include <QGSettings>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#define LIST_PATH "/etc/pairDevice.list"

class KDbusInterface;

class MainProgram : public QObject
{
    Q_OBJECT
public:
    explicit MainProgram(bool env, QObject *parent = nullptr);
    ~MainProgram();
    bool exit_flag = false;

private slots:
    void sendFiles(QString, QStringList);
    void gsettingsChangedSlot(const QString &);
    void sendTransferFilesMesgSlot(QStringList);
    void sendTransferDeviceMesgSlot(QString address);
    void activeConnectionSlot(QString address, QString name, QString type, int rssi, int timeout);
    void receiveFileSlot(QString address, QString name, QString filename, QString type, quint64 size);
    void displayPasskeySlot(QString dev, QString name, QString passkey);
    void requestConfirmationSlot(QString dev, QString name, QString passkey);
    void connectionErrorSlot(QString, int);

private:
    bool                        _mIntel;
    QTimer                      *replyTimer;
    QStringList                 selectedFiles;
    QMap<QString,QVariant>      _mGsetting;
    QList<QString>              *deviceAddressList = nullptr;
    QList<QString>              *deviceTypeList = nullptr;

    Config                      *_mConfig = nullptr;
    BluetoothDbus               *_sessionDbus = nullptr;
    TrayWidget                  *UI = nullptr;
    PinCodeWidget               *pincodewidget = nullptr;
    PinCodeWidget               *Keypincodewidget = nullptr;
    FileReceivingPopupWidget    *receiving_widget = nullptr;
    BluetoothFileTransferWidget *fileSendWidget = nullptr;
    ActiveConnectionWidget      *activeConnectionWidget = nullptr;
    ErrorMessageWidget          *errMsgWidget = nullptr;

    QString                     m_Passkey;
    QString                     current_User;
    QString                     active_User;

    void initGSettingInfo();
};

#endif // MAINPROGRAM_H

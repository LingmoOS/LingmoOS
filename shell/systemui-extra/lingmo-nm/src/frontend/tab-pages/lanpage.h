/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef LANPAGE_H
#define LANPAGE_H

#include "divider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QListWidget>
#include <QMap>
#include <QGSettings>

#include "list-items/listitem.h"
#include "list-items/lanlistitem.h"
#include "tab-pages/tabpage.h"

class LanListItem;

class LanPage : public TabPage
{
    Q_OBJECT
public:
    explicit LanPage(QWidget *parent = nullptr);
    ~LanPage();

    //for dbus
    void getWiredList(QString devName, QList<QStringList> &list);
    void activateWired(const QString& devName, const QString& connUuid);
    void deactivateWired(const QString& devName, const QString& connUuid);
    void showDetailPage(QString devName, QString uuid);
    void setWiredDeviceEnable(const QString& devName, bool enable);
    void deleteWired(const QString &connUuid);

    bool lanIsConnected();
    void getWiredDeviceConnectState(QMap<QString, QString> &map);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void initLanDevice();
    void initUI();
    void initLanArea();
    void initNetSwitch();
    void initLanDeviceState();

    void initDeviceCombox();
    void updateDeviceCombox(QString oldDeviceName, QString newDeviceName);
    void deleteDeviceFromCombox(QString deviceName);
    void addDeviceForCombox(QString deviceName);

    QListWidgetItem *insertNewItem(KyConnectItem *itemData, QListWidget *listWidget);
    QListWidgetItem *addNewItem(KyConnectItem *itemData, QListWidget *listWidget);
    bool removeConnectionItem(QMap<QString, QListWidgetItem *> &connectMap,
                              QListWidget *lanListWidget, QString path);

    void getEnabledDevice(QStringList &enableDeviceList);
    void getDisabledDevices(QStringList &disableDeviceList);

    void constructConnectionArea();
    void constructActiveConnectionArea();

    void updateConnectionArea(KyConnectItem *p_newItem);
    void updateActivatedConnectionArea(KyConnectItem *p_newItem);
    void updateConnectionState(QMap<QString, QListWidgetItem *> &connectMap,
                                        QListWidget *lanListWidget, QString uuid, ConnectState state);
    QString getConnectionDevice(QString uuid);

    void updateActiveConnectionProperty(KyConnectItem *p_connectItem);
    void updateConnectionProperty(KyConnectItem *p_connectItem);

    void sendLanUpdateSignal(KyConnectItem *p_connectItem);
    void sendLanAddSignal(KyConnectItem *p_connectItem);
    void sendLanStateChangeSignal(QString uuid, ConnectState state);

    void addEmptyConnectItem(QMap<QString, QListWidgetItem *> &connectMap,
                                           QListWidget *lanListWidget);
    void clearConnectionMap(QMap<QString, QListWidgetItem *> &connectMap,
                            QListWidget *lanListWidget);
    void deleteConnectionMapItem(QMap<QString, QListWidgetItem *> &connectMap,
                                 QListWidget *lanListWidget, QString uuid);

    void updateCurrentDevice(QString deviceName);
    void showRate();

Q_SIGNALS:
    void lanAdd(QString devName, QStringList info);
    void lanRemove(QString dbusPath);
    void lanUpdate(QString devName, QStringList info);

    void lanActiveConnectionStateChanged(QString interface, QString uuid, int status);
    void lanConnectChanged(int state);

    void showLanRate(QListWidget *widget, QMap<QString, QListWidgetItem *> &map, QString dev, bool isLan);

private Q_SLOTS:
    void onConnectionStateChange(QString uuid, NetworkManager::ActiveConnection::State state,
                                 NetworkManager::ActiveConnection::Reason reason);

    void onAddConnection(QString uuid);
    void onRemoveConnection(QString path);
    void onUpdateConnection(QString uuid);

    void onSwithGsettingsChanged(const QString &key);

    void onDeviceAdd(QString deviceName, NetworkManager::Device::Type deviceType);
    void onDeviceRemove(QString deviceName);
    void onDeviceNameUpdate(QString oldName, QString newName);
    void onDeviceManagedChange(QString deviceName, bool managed);

    void onDeviceCarriered(QString deviceName, bool pluged);
    void onDeviceActiveChanage(QString deviceName, bool deviceActive);

    void onDeviceComboxIndexChanged(int currentIndex);

    void onShowControlCenter();

    void onWiredEnabledChanged(bool);
private:
    QListWidget * m_activatedLanListWidget = nullptr;
    QListWidget * m_inactivatedLanListWidget = nullptr;

    KyNetworkDeviceResourse *m_deviceResource = nullptr;
    KyWiredConnectOperation *m_wiredConnectOperation = nullptr;
    KyActiveConnectResourse *m_activeResourse = nullptr;     //激活的连接
    KyConnectResourse *m_connectResourse = nullptr;          //未激活的连接 

    QMap<QString, QListWidgetItem *> m_inactiveConnectionMap;
    QMap<QString, QListWidgetItem *> m_activeConnectionMap;

    QString m_currentDeviceName;
    QStringList m_devList;
    QStringList m_enableDeviceList;
    QStringList m_disableDeviceList;

    QGSettings *m_switchGsettings = nullptr;
    QMap<QString, NetDetail*> m_lanPagePtrMap;
};

#endif // LANPAGE_H

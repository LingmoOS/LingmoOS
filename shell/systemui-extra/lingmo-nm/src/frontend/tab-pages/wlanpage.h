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
#ifndef WLANPAGE_H
#define WLANPAGE_H

#include "tabpage.h"
#include "kywirelessnetresource.h"
#include "lingmoactiveconnectresource.h"
#include "lingmonetworkdeviceresource.h"
#include "kywirelessconnectoperation.h"
#include "wlanlistitem.h"
#include "wlanmoreitem.h"
#include "lingmoconnectoperation.h"
#include <QGSettings>
#include "netdetails/netdetail.h"
#include <QProcess>
#include "lingmoactiveconnectresource.h"
#include "kywirelessnetresource.h"
#include "netdetails/joinhiddenwifipage.h"

//#define SCROLLAREA_HEIGHT 150
#define MORE_TEXT_MARGINS 16,0,0,0
#define SCROLLAREA_HEIGHT 200

#define LAN_PAGE_INDEX 0
#define WLAN_PAGE_INDEX 1

class WlanListItem;

class WlanPage : public TabPage
{
    Q_OBJECT
public:
    explicit WlanPage(QWidget *parent = nullptr);
    ~WlanPage() = default;

    //for dbus
    void getWirelessList(QString devName, QList<QStringList> &list);
    //开启热点
    void activeWirelessAp(const QString apName, const QString apPassword, const QString wirelessBand, const QString apDevice);
    //断开热点
    void deactiveWirelessAp(const QString apName, const QString uuid);
    //获取热点
    void getStoredApInfo(QStringList &list);
    void getApConnectionPath(QString &path, QString uuid);
    void getActiveConnectionPath(QString &path, QString uuid);

    void activateWirelessConnection(const QString& devName, const QString& ssid);
    void deactivateWirelessConnection(const QString& devName, const QString& ssid);

    void showDetailPage(QString devName, QString uuid);
    void showAddOtherPage(QString devName);

    bool checkWlanStatus(NetworkManager::ActiveConnection::State state);

    void getApInfoBySsid(QString devName, QString ssid, QStringList &list);
    //无线总开关
    void setWirelessSwitchEnable(bool enable);
    void getWirelessDeviceCap(QMap<QString, int> &map);

    void getConnectivity(NetworkManager::Connectivity &connectivity);

    bool getWirelessSwitchBtnState();

    int getActivateWifiSignal(QString devName = "");

    //无线网卡连通性
    void getWirelssDeviceConnectState(QMap<QString, QString> &map);

    QString getCurrentDisplayDevice() {
        return m_currentDevice;
    }

Q_SIGNALS:
    void oneItemExpanded(const QString &ssid);
    void wlanAdd(QString devName, QStringList info);
    void wlanRemove(QString devName,QString ssid);
    void wlanActiveConnectionStateChanged(QString interface, QString ssid, QString uuid, int status);
    void hotspotDeactivated(QString devName, QString ssid);
    void hotspotActivated(QString devName, QString ssid, QString uuid, QString activePath, QString settingPath);
    void signalStrengthChange(QString devName, QString ssid, int strength);
    void secuTypeChange(QString devName, QString ssid, QString secuType);
    void hiddenWlanClicked();
    void wlanConnectChanged(int state);
    void timeToUpdate();

    void showMainWindow(int type);

    void connectivityChanged(NetworkManager::Connectivity connectivity);
    void connectivityCheckSpareUriChanged();

    void wirelessSwitchBtnChanged(bool state);

public Q_SLOTS:
    void onMainWindowVisibleChanged(const bool &visible);
    void onSecurityTypeChange(QString devName, QString ssid, QString secuType);
    void requestScan();
    void onWlanPageVisibleChanged(int index);

private Q_SLOTS:
    void onWlanAdded(QString interface, KyWirelessNetItem &item);
    void onWlanRemoved(QString interface, QString ssid);

    void onConnectionAdd(QString deviceName, QString ssid);
    void onConnectionRemove(QString deviceName, QString ssid, QString path);
    void onConnectionUpdate(QString deviceName, QString ssid);

    void onDeviceAdd(QString deviceName, NetworkManager::Device::Type deviceType);
    void onDeviceRemove(QString deviceName);
    void onDeviceNameUpdate(QString oldName, QString newName);

    void onConnectionStateChanged(QString uuid,
                                NetworkManager::ActiveConnection::State state,
                                NetworkManager::ActiveConnection::Reason reason);
    void onItemHeightChanged(const bool isExpanded, const QString &ssid);

    void onDeviceComboxIndexChanged(int currentIndex);
    void onHiddenWlanClicked();
    void showControlCenter();
    void onWifiEnabledChanged(bool isWifiOn);
    void onRefreshIconTimer();

    void onWlanStateChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);
    void onDeviceManagedChanged(QString deviceName, bool managed);

    void onInactivateListWidgetItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void setInactivateListItemNoSelect();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    //定时触发扫描的定时器
    void initTimer();
    void initWlanUI();
    void initConnections();
    void initDevice();//初始化默认设备
    void initDeviceCombox();
    void initWlanSwitchState();
    void initWlanArea();
    void addWlanMoreItem();

    void showNonePwd();

    void showRate();

    void showHiddenWlanPage(QString devName);

    QListWidgetItem *addEmptyItem(QListWidget *wirelessListWidget);
    QListWidgetItem *addNewItem(KyWirelessNetItem &wirelessNetItem,
                                          QListWidget *wirelessListWidget);
    QListWidgetItem *insertNewItem(KyWirelessNetItem &wirelessNetItem,
                                             QListWidget *wirelessListWidget,
                                             int row);
    QListWidgetItem *insertNewItemWithSort(KyWirelessNetItem &wirelessNetItem,
                                             QListWidget *p_ListWidget);

    void clearWirelessNetItemMap(QMap<QString, QListWidgetItem*> &wirelessNetItem,
                                           QListWidget *wirelessListWidget);
    void deleteWirelessItemFormMap(QMap<QString, QListWidgetItem*> &wirelessNetItemMap,
                                             QListWidget *wirelessListWidget, QString ssid);

    void updateWlanItemState(QListWidget *p_wirelessListWidget,
                                       QListWidgetItem *p_listWidgetItem,
                                       ConnectState state);
    void updateWlanListItem(QString ssid);
    void refreshActiveConnectionIcon(QString ssid, const int &signal);

    void constructWirelessNetArea();
    void constructActivateConnectionArea();

    void updateActivatedArea(QString uuid, QString ssid, QString devName);
    void updateWirelessNetArea(QString uuid, QString ssid, QString devName, QString path);

    void addDeviceToCombox(QString deviceName);
    void deleteDeviceFromCombox(QString deviceName);
    void updateDeviceForCombox(QString oldDeviceName, QString newDeviceName);

    void sendApStateChangeSignal(QString uuid, QString ssid, QString deviceName,
                                     NetworkManager::ActiveConnection::State state);
//    void wlanShowNotify(QString ssid, NetworkManager::ActiveConnection::State state,
//                                  NetworkManager::ActiveConnection::Reason reason);

    //是否存在可用的无线网卡
    bool getWirelessDevieceUseable();
    void setWirelessEnable(bool state);
    bool getWirelessEnable();
    inline void setSwitchBtnState(bool state) {
        if (m_netSwitch != nullptr) {
            m_netSwitch->setChecked(state);
        }
    }
    inline bool getSwitchBtnState() {
        if (m_netSwitch != nullptr) {
            return m_netSwitch->isChecked();
        }
    }
    inline void setSwitchBtnEnable(bool state) {
        if (m_netSwitch != nullptr) {
            m_netSwitch->setEnabled(state);
        }
    }
    inline bool getSwitchBtnEnable() {
        if (m_netSwitch != nullptr) {
            return m_netSwitch->isEnabled();
        }
    }

    void checkShowWifi6Plus();
    bool m_showWifi6Plus = true;

private:
    QMap<QString, QListWidgetItem*> m_wirelessNetItemMap;
    QMap<QString, QListWidgetItem*> m_activateConnectionItemMap;

    QListWidgetItem *m_expandedItem = nullptr;

    WlanMoreItem * m_hiddenWlanWidget = nullptr;
    QListWidgetItem *m_hiddenItem = nullptr;

    QListWidget * m_activatedNetListWidget = nullptr;
    QListWidget * m_inactivatedNetListWidget = nullptr;

    QStringList m_devList;
    QString m_currentDevice;

    KyWirelessNetResource *m_wirelessNetResource = nullptr;
    KyActiveConnectResourse *m_activatedConnectResource = nullptr;
    KyNetworkDeviceResourse *m_netDeviceResource = nullptr;
    KyWirelessConnectOperation * m_wirelessConnectOpreation = nullptr;
    KyConnectResourse * m_connectResource = nullptr;

    bool m_updateStrength = true;

    QTimer *m_scanTimer = nullptr;
    QTimer *m_refreshIconTimer = nullptr;

    QMap<QString, QMap<QString, NetDetail*>> m_wlanPagePtrMap;
    QMap<QString, JoinHiddenWiFiPage*> m_joinHiddenWiFiPagePtrMap;
};

#endif // WLANPAGE_H

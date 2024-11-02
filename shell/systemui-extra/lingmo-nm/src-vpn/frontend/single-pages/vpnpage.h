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
#ifndef VPNPAGE_H
#define VPNPAGE_H

#include "divider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QListWidget>
#include <QMap>
#include <QGSettings>

#include <KWindowSystem>

#include "list-items/listitem.h"
#include "list-items/vpnlistitem.h"
#include "single-pages/singlepage.h"

#define VPNPAGE_LAYOUT_MARGINS 0,0,0,0
#define VPN_LIST_SPACING 0
#define ITEM_HEIGHT 50
#define ITEM_SPACE 8
#define PAGE_SPACE 22

#define LOG_FLAG "[VpnPage]"

#define VISIBLE "visible"
const QByteArray GSETTINGS_VPNICON_VISIBLE = "org.lingmo.lingmo-nm.vpnicon";

class VpnListItem;

class VpnPage : public SinglePage
{
    Q_OBJECT
public:
    explicit VpnPage(QWidget *parent = nullptr);
    ~VpnPage();

    //for dbus
    void getVirtualList(QVector<QStringList> &vector);
    void deleteVpn(const QString &connUuid);
    void activateVpn(const QString& connUuid);
    void deactivateVpn(const QString& connUuid);
    void showDetailPage(QString uuid);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void initPanelGSettings();
    void initUI();
    void initVpnArea();
    void resetPageHeight();

    inline void initDeviceCombox() { return; }

    QListWidgetItem *insertNewItem(KyConnectItem *itemData, QListWidget *listWidget);
    QListWidgetItem *addNewItem(KyConnectItem *itemData, QListWidget *listWidget);
    bool removeConnectionItem(QMap<QString, QListWidgetItem *> &connectMap,
                              QListWidget *vpnListWidget, QString path);

    void constructItemArea();

    void updateConnectionArea(KyConnectItem *p_newItem);
    void updateActivatedConnectionArea(KyConnectItem *p_newItem);
    void updateConnectionState(QMap<QString, QListWidgetItem *> &connectMap,
                                        QListWidget *vpnListWidget, QString uuid, ConnectState state);

    void updateActiveConnectionProperty(KyConnectItem *p_connectItem);
    void updateConnectionProperty(KyConnectItem *p_connectItem);

    void sendVpnUpdateSignal(KyConnectItem *p_connectItem);
    void sendVpnAddSignal(KyConnectItem *p_connectItem);
    void sendVpnStateChangeSignal(QString uuid, ConnectState state);

    void clearConnectionMap(QMap<QString, QListWidgetItem *> &connectMap,
                            QListWidget *vpnListWidget);
    void deleteConnectionMapItem(QMap<QString, QListWidgetItem *> &connectMap,
                                 QListWidget *vpnListWidget, QString uuid);

    void resetWindowPosition();
    void resetListWidgetWidth();

Q_SIGNALS:
    void vpnAdd(QStringList info);
    void vpnRemove(QString dbusPath);
    void vpnUpdate(QStringList info);

    void vpnActiveConnectionStateChanged(QString uuid, int status);
    void vpnConnectChanged(int state);

private Q_SLOTS:
    void onConnectionStateChange(QString uuid,
                                 NetworkManager::ActiveConnection::State state,
                                 NetworkManager::ActiveConnection::Reason reason);

    void onAddConnection(QString uuid);
    void onRemoveConnection(QString path);
    void onUpdateConnection(QString uuid);

    void onShowControlCenter();

private:
    KyVpnConnectOperation *m_vpnConnectOperation = nullptr;
    KyActiveConnectResourse *m_activeResourse = nullptr;     //激活的连接
    KyConnectResourse *m_connectResourse = nullptr;          //未激活的连接

    QMap<QString, QListWidgetItem *> m_vpnItemMap;
    QMap<QString, QListWidgetItem *> m_activeItemMap;


    //获取任务栏位置和大小
    QGSettings *m_panelGSettings = nullptr;
    int m_panelPosition;
    int m_panelSize;


public Q_SLOTS:
    void showUI();
};

#endif // LANPAGE_H

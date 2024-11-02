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
#include "vpnpage.h"
#include <QDebug>
#include <QScrollBar>
#include <QScreen>
#include "windowmanager/windowmanager.h"

#define PANEL_SETTINGS "org.lingmo.panel.settings"
#define PANEL_SIZE_KEY "panelsize"
#define PANEL_POSITION_KEY "panelposition"

VpnPage::VpnPage(QWidget *parent) : SinglePage(parent)
{
    m_activeResourse = new KyActiveConnectResourse(this);
    m_connectResourse = new KyConnectResourse(this);
    m_vpnConnectOperation = new KyVpnConnectOperation(this);

    initPanelGSettings();
    initUI();
    initVpnArea();
    installEventFilter(this);

    connect(m_activeResourse, &KyActiveConnectResourse::stateChangeReason, this, &VpnPage::onConnectionStateChange);
    connect(m_activeResourse, &KyActiveConnectResourse::activeConnectRemove, this, [=] (QString activeConnectUuid) {
        sendVpnStateChangeSignal(activeConnectUuid,Deactivated);
    } );

    connect(m_connectResourse, &KyConnectResourse::connectionAdd, this, &VpnPage::onAddConnection);
    connect(m_connectResourse, &KyConnectResourse::connectionRemove, this, &VpnPage::onRemoveConnection);
    connect(m_connectResourse, &KyConnectResourse::connectionUpdate, this, &VpnPage::onUpdateConnection);

    connect(m_vpnConnectOperation, &KyVpnConnectOperation::activateConnectionError, this, &VpnPage::activateFailed);
    connect(m_vpnConnectOperation, &KyVpnConnectOperation::deactivateConnectionError, this, &VpnPage::deactivateFailed);

//    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, [&](WId activeWindowId){
//        if (activeWindowId != this->winId() && activeWindowId != 0) {
//            hide();
//        }
//    });
}

VpnPage::~VpnPage()
{

}

void VpnPage::deleteConnectionMapItem(QMap<QString, QListWidgetItem *> &connectMap,
                             QListWidget *vpnListWidget, QString uuid)
{
    QListWidgetItem *p_listWidgetItem = connectMap.value(uuid);
    if (p_listWidgetItem) {
        connectMap.remove(uuid);
        VpnListItem *p_vpnItem = (VpnListItem *)vpnListWidget->itemWidget(p_listWidgetItem);
        vpnListWidget->removeItemWidget(p_listWidgetItem);

        delete p_vpnItem;
        p_vpnItem = nullptr;

        delete p_listWidgetItem;
        p_listWidgetItem = nullptr;
    }

    return;
}

void VpnPage::clearConnectionMap(QMap<QString, QListWidgetItem *> &connectMap,
                                 QListWidget *vpnListWidget)
{
    QMap<QString, QListWidgetItem *>::iterator iter;

    iter = connectMap.begin();
    while (iter != connectMap.end()) {
        qDebug()<<"[VpnPage] clear connection map item"<< iter.key();

        QListWidgetItem *p_widgetItem = iter.value();
        VpnListItem *p_vpnItem = (VpnListItem *)vpnListWidget->itemWidget(p_widgetItem);
        vpnListWidget->removeItemWidget(p_widgetItem);

        delete p_vpnItem;
        p_vpnItem = nullptr;

        delete p_widgetItem;
        p_widgetItem = nullptr;

        iter = connectMap.erase(iter);
    }

    return;
}

void VpnPage::constructItemArea()
{
    QList<KyConnectItem *> activedList;
    QList<KyConnectItem *> netList;

    activedList.clear();
    netList.clear();
    clearConnectionMap(m_activeItemMap, m_listWidget);
    clearConnectionMap(m_vpnItemMap, m_listWidget);

    m_connectResourse->getVpnAndVirtualConnections(netList);
    KyConnectItem *p_newItem = nullptr;
    if (!netList.isEmpty()) {
        for (int index = 0; index < netList.size(); index++) {
            KyConnectItem *p_netConnectionItem = netList.at(index);
            p_newItem = m_activeResourse->getActiveConnectionByUuid(p_netConnectionItem->m_connectUuid);
            if (p_newItem == nullptr) {
                if (m_vpnItemMap.contains(p_netConnectionItem->m_connectUuid)) {
                    qDebug()<<LOG_FLAG << "has contain uuid" << p_netConnectionItem->m_connectUuid;
                }
                QListWidgetItem *p_listWidgetItem = addNewItem(p_netConnectionItem, m_listWidget);
                m_vpnItemMap.insert(p_netConnectionItem->m_connectUuid, p_listWidgetItem);
            } else {
                if (m_activeItemMap.contains(p_netConnectionItem->m_connectUuid)) {
                    qDebug()<<LOG_FLAG << "has contain uuid" << p_netConnectionItem->m_connectUuid;
                }
                QListWidgetItem *p_listWidgetItem = addNewItem(p_newItem, m_listWidget);
                m_activeItemMap.insert(p_netConnectionItem->m_connectUuid, p_listWidgetItem);
            }

            delete p_netConnectionItem;
            p_netConnectionItem = nullptr;
        }
    }

    if (QGSettings::isSchemaInstalled(GSETTINGS_VPNICON_VISIBLE)) {
        QGSettings vpnGsettings(GSETTINGS_VPNICON_VISIBLE);
        if (vpnGsettings.keys().contains(QString(VISIBLE))) {
            if (!netList.isEmpty()) {
                vpnGsettings.set(VISIBLE, true);
            } else {
                vpnGsettings.set(VISIBLE, false);
            }
        }
    }

    resetListWidgetWidth();
}

void VpnPage::initVpnArea()
{
    constructItemArea();
}

void VpnPage::resetPageHeight()
{
    int height = 0;
    int count = m_listWidget->count();
    m_listFrame->setFixedHeight((count >= 4) ? (MAX_ITEMS * ITEM_HEIGHT + ITEM_SPACE) : (count * ITEM_HEIGHT + ITEM_SPACE));

    if (count == 0) {
        m_listWidget->setHidden(true);
        m_listFrame->setHidden(true);
        m_netDivider->setHidden(true);
    } else {
        m_listWidget->show();
        m_listFrame->show();
        m_netDivider->show();
    }
}

bool VpnPage::removeConnectionItem(QMap<QString, QListWidgetItem *> &connectMap,
                          QListWidget *vpnListWidget, QString path)
{
    QMap<QString, QListWidgetItem *>::iterator iter;
    for (iter = connectMap.begin(); iter != connectMap.end(); ++iter) {
        QListWidgetItem *p_listWidgetItem = iter.value();
        VpnListItem *p_vpnItem = (VpnListItem*)vpnListWidget->itemWidget(p_listWidgetItem);
        if (p_vpnItem->getConnectionPath() == path) {
            qDebug()<<"[VpnPage] Remove a connection from list";

            vpnListWidget->removeItemWidget(p_listWidgetItem);

            delete p_vpnItem;
            p_vpnItem = nullptr;

            delete p_listWidgetItem;
            p_listWidgetItem = nullptr;

            iter = connectMap.erase(iter);
            resetListWidgetWidth();
            return true;
        }
    }

    return false;
}

void VpnPage::onRemoveConnection(QString path)            //删除时后端会自动断开激活，将其从未激活列表中删除
{
    //for dbus
    qDebug() << "[VpnPage] emit lanRemove because onRemoveConnection " << path;
    Q_EMIT vpnRemove(path);

    removeConnectionItem(m_vpnItemMap, m_listWidget, path);
    removeConnectionItem(m_activeItemMap, m_listWidget, path);
    resetPageHeight();
    resetWindowPosition();
    this->update();
}

void VpnPage::onAddConnection(QString uuid)               //新增一个有线连接，将其加入到激活列表
{
    if (!m_connectResourse->isVirtualConncection(uuid)) {
        return;
    }

    KyConnectItem *p_newItem = nullptr;
    p_newItem = m_connectResourse->getConnectionItemByUuid(uuid);
    if (nullptr == p_newItem) {
        return;
    }

    sendVpnAddSignal(p_newItem);

    qDebug()<<"[VpnPage] Add a new connection, name:"<<p_newItem->m_connectName;
    QListWidgetItem *p_listWidgetItem = insertNewItem(p_newItem, m_listWidget);
    if (m_vpnItemMap.contains(p_newItem->m_connectUuid)) {
        qDebug()<<LOG_FLAG << "the connection is exist" << p_newItem->m_connectUuid;
    }
    m_vpnItemMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);

    delete p_newItem;
    p_newItem = nullptr;
    resetListWidgetWidth();
    resetPageHeight();
    resetWindowPosition();
    this->update();
    return;
}

void VpnPage::onShowControlCenter()
{
    QProcess process;
    process.startDetached("lingmo-control-center -m vpn");
}

void VpnPage::initUI()
{
    m_titleLabel->setText(tr("VPN"));

//    m_listFrame->setMaximumHeight(MAX_ITEMS * ITEM_HEIGHT + ITEM_SPACE);

    m_listWidget->setFrameShape(QFrame::Shape::NoFrame);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    m_listWidget->verticalScrollBar()->setProperty("drawScrollBarGroove",false); //去除滚动条的外侧黑框
    m_listWidget->verticalScrollBar()->setSingleStep(SCROLL_STEP);
    m_listWidget->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    m_settingsLabel->setText(tr("VPN Settings"));
    m_settingsLabel->installEventFilter(this);
}

QListWidgetItem *VpnPage::insertNewItem(KyConnectItem *itemData, QListWidget *listWidget)
{
    int index = 0;

    for(index = 0; index < m_listWidget->count(); index++) {
        QListWidgetItem *p_listWidgetItem = m_listWidget->item(index);
        VpnListItem *p_vpnItem = (VpnListItem *)m_listWidget->itemWidget(p_listWidgetItem);
        QString name1 = p_vpnItem->getConnectionName();
        QString name2 = itemData->m_connectName;
        if (QString::compare(name1, name2, Qt::CaseInsensitive) > 0) {
            break;
        }
    }

    QListWidgetItem *p_sortListWidgetItem = new QListWidgetItem();
    p_sortListWidgetItem->setFlags(p_sortListWidgetItem->flags() & (~Qt::ItemIsSelectable));   //设置不可被选中
    p_sortListWidgetItem->setSizeHint(QSize(listWidget->width(),ITEM_HEIGHT));

    listWidget->insertItem(index, p_sortListWidgetItem);

    VpnListItem *p_sortLanItem = nullptr;
    p_sortLanItem = new VpnListItem(itemData);
    listWidget->setItemWidget(p_sortListWidgetItem, p_sortLanItem);

    return p_sortListWidgetItem;
}

QListWidgetItem *VpnPage::addNewItem(KyConnectItem *itemData, QListWidget *listWidget)
{
    QListWidgetItem *p_listWidgetItem = new QListWidgetItem();
    p_listWidgetItem->setFlags(p_listWidgetItem->flags() & (~Qt::ItemIsSelectable));
    p_listWidgetItem->setSizeHint(QSize(listWidget->width() - 16, ITEM_HEIGHT));
    listWidget->addItem(p_listWidgetItem);
    VpnListItem *p_vpnItem = nullptr;
    if (itemData != nullptr) {
        p_vpnItem = new VpnListItem(itemData);
        qDebug() << "[VpnPage] addNewItem, connection: " << itemData->m_connectName;
    } else {
        p_vpnItem = new VpnListItem();
        qDebug() << "[VpnPage] Add nullItem!";
    }

    listWidget->setItemWidget(p_listWidgetItem, p_vpnItem);
    return p_listWidgetItem;
}

void VpnPage::updateActivatedConnectionArea(KyConnectItem *p_newItem)
{
    if (m_activeItemMap.contains(p_newItem->m_connectUuid)) {
        return;
    }

    deleteConnectionMapItem(m_vpnItemMap, m_listWidget, p_newItem->m_connectUuid);
    qDebug()<<"[VpnPage]update active connection item"<<p_newItem->m_connectName;
    deleteConnectionMapItem(m_activeItemMap, m_listWidget, p_newItem->m_connectUuid);
    QListWidgetItem *p_listWidgetItem = addNewItem(p_newItem, m_listWidget);
    m_activeItemMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);

    resetListWidgetWidth();

    return;
}

void VpnPage::updateConnectionArea(KyConnectItem *p_newItem)
{
    if (m_vpnItemMap.contains(p_newItem->m_connectUuid)) {
        return;
    }

    deleteConnectionMapItem(m_activeItemMap, m_listWidget, p_newItem->m_connectUuid);
    qDebug()<<"[VpnPage] update connection item"<<p_newItem->m_connectName;
    QListWidgetItem *p_listWidgetItem = insertNewItem(p_newItem, m_listWidget);
    m_vpnItemMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);

    resetListWidgetWidth();
}

void VpnPage::updateConnectionState(QMap<QString, QListWidgetItem *> &connectMap,
                                    QListWidget *vpnListWidget, QString uuid, ConnectState state)
{
    qDebug() << LOG_FLAG << "update connection state";

    QListWidgetItem *p_listWidgetItem = connectMap.value(uuid);
    if (p_listWidgetItem) {
        VpnListItem *p_vpnItem = (VpnListItem *)vpnListWidget->itemWidget(p_listWidgetItem);
        p_vpnItem->updateConnectionState(state);
    }

}

void VpnPage::onConnectionStateChange(QString uuid,
                                      NetworkManager::ActiveConnection::State state,
                                      NetworkManager::ActiveConnection::Reason reason)
{
    //VpnPage函数内持续监听连接状态的变化并记录供其他函数调用获取状态
    if (!m_connectResourse->isVirtualConncection(uuid)) {
        qDebug() << "[VpnPage] connection state change signal but not vpn";
        return;
    }

    sendVpnStateChangeSignal(uuid, (ConnectState)state);

    if (m_activeItemMap.keys().contains(uuid) && state == NetworkManager::ActiveConnection::State::Activated) {
        return;
    }

    qDebug()<<"[VpnPage] connection uuid"<< uuid
            << "state change slot:"<< state;

    KyConnectItem *p_newItem = nullptr;
    QString deviceName = "";
    QString ssid = "";

    if (state == NetworkManager::ActiveConnection::State::Activated) {
        p_newItem = m_activeResourse->getActiveConnectionByUuid(uuid);
        if (nullptr == p_newItem) {
            qWarning()<<"[VpnPage] get active connection failed, connection uuid" << uuid;
            return;
        }

        ssid = p_newItem->m_connectName;
        updateActivatedConnectionArea(p_newItem);
        updateConnectionState(m_activeItemMap, m_listWidget, uuid, (ConnectState)state);
    } else if (state == NetworkManager::ActiveConnection::State::Deactivated) {
        p_newItem = m_connectResourse->getConnectionItemByUuid(uuid);
        qDebug() << "[VpnPage] deactivated reason" << reason;
        if (nullptr == p_newItem) {
            qWarning()<<"[VpnPage] get active connection failed, connection uuid" << uuid;
            return;
        }

        ssid = p_newItem->m_connectName;
        updateConnectionArea(p_newItem);
        updateConnectionState(m_vpnItemMap, m_listWidget, uuid, (ConnectState)state);
    } else if (state == NetworkManager::ActiveConnection::State::Activating) {
        updateConnectionState(m_vpnItemMap, m_listWidget, uuid, (ConnectState)state);
    } else if (state == NetworkManager::ActiveConnection::State::Deactivating) {
        updateConnectionState(m_activeItemMap, m_listWidget, uuid, (ConnectState)state);
    }

    Q_EMIT vpnActiveConnectionStateChanged(uuid, state);

    if (p_newItem) {
        delete p_newItem;
        p_newItem = nullptr;
    }

    return;
}

void VpnPage::getVirtualList(QVector<QStringList> &vector)
{
    QList<KyConnectItem *> netConnectList;
    vector.clear();
    m_connectResourse->getVpnAndVirtualConnections(netConnectList);      //未激活列表的显示
    if (!netConnectList.isEmpty()) {
        for (int i = 0; i < netConnectList.size(); i++) {
            KyConnectItem *p_newItem = nullptr;
            KyConnectItem *p_netConnectionItem = netConnectList.at(i);
            p_newItem = m_activeResourse->getActiveConnectionByUuid(p_netConnectionItem->m_connectUuid);
            NetworkManager::ActiveConnection::State state = p_netConnectionItem->m_connectState;
            if (p_newItem != nullptr) {
                state = NetworkManager::ActiveConnection::Activated;
            }
            vector.append(QStringList() << netConnectList.at(i)->m_connectName
                                        << netConnectList.at(i)->m_connectUuid
                                        << netConnectList.at(i)->m_connectPath
                                        << QString::number(state));
        }
    }
    return;
}

void VpnPage::sendVpnUpdateSignal(KyConnectItem *p_connectItem)
{
    QStringList info;
    info << p_connectItem->m_connectName << p_connectItem->m_connectUuid << p_connectItem->m_connectPath;
    Q_EMIT vpnUpdate(info);

    return;
}

void VpnPage::sendVpnAddSignal(KyConnectItem *p_connectItem)
{
    QStringList info;
    KyConnectItem *p_newItem = nullptr;
    p_newItem = m_activeResourse->getActiveConnectionByUuid(p_connectItem->m_connectUuid);
    NetworkManager::ActiveConnection::State state = p_connectItem->m_connectState;
    if (p_newItem != nullptr) {
        state = NetworkManager::ActiveConnection::Activated;
    }
    info << p_connectItem->m_connectName
         << p_connectItem->m_connectUuid
         << p_connectItem->m_connectPath
         << QString::number(state);
    qDebug() << "[VpnPage] emit vpnAdd because addConnection ";
    Q_EMIT vpnAdd(info);

    return;
}

void VpnPage::sendVpnStateChangeSignal(QString uuid, ConnectState state)
{
    if (state == Activating || state == Deactivating) {
        if (m_activeResourse->connectionIsVirtual(uuid)) {
            return;
        }
    }

    Q_EMIT this->vpnConnectChanged(state);

    return;
}

void VpnPage::updateConnectionProperty(KyConnectItem *p_connectItem)
{
    QString newUuid = p_connectItem->m_connectUuid;

    if (m_vpnItemMap.contains(newUuid)) {
        QListWidgetItem *p_listWidgetItem = m_vpnItemMap.value(newUuid);
        VpnListItem *p_vpnItem = (VpnListItem*)m_listWidget->itemWidget(p_listWidgetItem);
        if (p_connectItem->m_connectName != p_vpnItem->getConnectionName()){
            //只要名字改变就要删除，重新插入，主要是为了排序
            deleteConnectionMapItem(m_vpnItemMap, m_listWidget, newUuid);
            QListWidgetItem *p_sortListWidgetItem = insertNewItem(p_connectItem, m_listWidget);
            if (m_vpnItemMap.contains(newUuid)) {
                qDebug()<<LOG_FLAG << "has contained connection" << newUuid;
            }
            m_vpnItemMap.insert(newUuid, p_sortListWidgetItem);
        } else if (p_connectItem->m_connectPath != p_vpnItem->getConnectionPath()) {
            p_vpnItem->updateConnectionPath(p_connectItem->m_connectPath);
        }
    } else if (!m_activeItemMap.contains(newUuid)){
        if (p_connectItem->m_ifaceName.isEmpty()) {
            QListWidgetItem *p_listWidgetItem = insertNewItem(p_connectItem, m_listWidget);
            if (m_vpnItemMap.contains(newUuid)) {
                qDebug()<<LOG_FLAG << "has contained connection uuid" << newUuid;
            }
            m_vpnItemMap.insert(newUuid, p_listWidgetItem);
        }
    } else {
        qWarning() << LOG_FLAG << newUuid <<" is in activemap, so not process.";
    }

    return;
}

void VpnPage::updateActiveConnectionProperty(KyConnectItem *p_connectItem)
{
    QString newUuid = p_connectItem->m_connectUuid;

    if (m_activeItemMap.contains(newUuid)) {
        QListWidgetItem *p_listWidgetItem = m_activeItemMap.value(newUuid);
        VpnListItem *p_vpnItem = (VpnListItem *)m_listWidget->itemWidget(p_listWidgetItem);
        if (p_vpnItem->getConnectionName() != p_connectItem->m_connectName) {
            p_vpnItem->updateConnectionName(p_connectItem->m_connectName);
        }

        if (p_vpnItem->getConnectionName() != p_connectItem->m_connectPath) {
            p_vpnItem->updateConnectionPath(p_connectItem->m_connectPath);
        }
    }
    return;
}

void VpnPage::onUpdateConnection(QString uuid)
{
    if (!m_connectResourse->isVirtualConncection(uuid)) {
        return;
    }

    qDebug() << "[VpnPage]:Connection property Changed." << Q_FUNC_INFO << __LINE__;

    KyConnectItem *p_newItem = nullptr;
    if (m_connectResourse->isActivatedConnection(uuid)) {
        p_newItem = m_activeResourse->getActiveConnectionByUuid(uuid);
        if (nullptr == p_newItem) {
            qWarning()<<"[VpnPage] get item failed, when update activate connection."
                      <<"connection uuid" << uuid;
            return;
        }

        updateActiveConnectionProperty(p_newItem);
    } else {
        p_newItem = m_connectResourse->getConnectionItemByUuid(uuid);
        if (nullptr == p_newItem) {
            qWarning()<<"[VpnPage] get item failed, when update connection."
                      <<"connection uuid"<<uuid;
            return;
        }

        updateConnectionProperty(p_newItem);
    }

    sendVpnUpdateSignal(p_newItem);

    delete p_newItem;
    p_newItem = nullptr;

    return;
}

bool VpnPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_settingsLabel) {
        if (event->type() == QEvent::MouseButtonRelease) {
            onShowControlCenter();
        }
    }

    if (watched == this) {
        //失焦退出
        if (event->type() == QEvent::ActivationChange) {
            if (QApplication::activeWindow() != this) {
                hide();
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void VpnPage::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        if (m_panelGSettings == nullptr) {
            m_panelGSettings = new QGSettings(id, QByteArray(), this);
        }
        if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        }
        if (m_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
            m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        }
        connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString &key) {
            if (key == PANEL_POSITION_KEY) {
                m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
            }
            if (key == PANEL_SIZE_KEY) {
                m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
            }
            if (this->isVisible()) {
                resetWindowPosition();
            }
        });
    }
}

void VpnPage::deleteVpn(const QString &connUuid)
{
    qDebug() << "[VpnPage] deleteVpn" << connUuid;
    if (connUuid == nullptr) {
        return;
    }
    m_vpnConnectOperation->deleteVpnConnect(connUuid);
}

void VpnPage::activateVpn(const QString& connUuid)
{
    if (m_vpnItemMap.contains(connUuid)) {
        qDebug() << "[VpnPage] activateVpn" << connUuid;
        m_vpnConnectOperation->activateVpnConnection(connUuid);
    }
}

void VpnPage::deactivateVpn(const QString& connUuid)
{
    qDebug() << "[VpnPage] deactivateVpn" << connUuid;
    QString name("");
    m_vpnConnectOperation->deactivateVpnConnection(name, connUuid);
}

void VpnPage::showDetailPage(QString uuid)
{
    QListWidgetItem * vpnlistItem = m_vpnItemMap.value(uuid);
    VpnListItem *vpnItem = (VpnListItem *)m_listWidget->itemWidget(vpnlistItem);
    vpnItem->onInfoButtonClicked();
}

void VpnPage::showUI()
{
    //2209中窗管在hide界面时会刷新属性，需要重新设置无图标属性
//    const KWindowInfo info(this->winId(), NET::WMState);
//    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager)) {
//        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
//    }

    resetPageHeight();

    showNormal();
    QWindow* window = this->windowHandle();
    if (window) {
        //跳过任务栏和分页器的属性
        kdk::WindowManager::setSkipSwitcher(window, true);
        kdk::WindowManager::setSkipTaskBar(window, true);
    }
    raise();
    activateWindow();
    resetWindowPosition();
    return;
}

void VpnPage::resetWindowPosition()
{
#define MARGIN 8
#define PANEL_TOP 1
#define PANEL_LEFT 2
#define PANEL_RIGHT 3
//#define PANEL_BOTTOM 4

    QRect availableGeo = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int x, y;
    char *envStr = getenv("LANGUAGE");
    switch(m_panelPosition){
    case PANEL_TOP:
        //任务栏位于上方
        /* 维吾尔语 ug_CN
         * 哈萨克语 kk_KZ
         * 柯尔克孜语 ky_KG */
        if (strcmp(envStr, "ug_CN") == 0 || strcmp(envStr, "kk_KZ") == 0 || strcmp(envStr, "ky_KG") == 0) {
            x = MARGIN;
            y = availableGeo.y() + m_panelSize + MARGIN;
        } else {
            x = availableGeo.x() + availableGeo.width() - this->width() - MARGIN;
            y = availableGeo.y() + m_panelSize + MARGIN;
        }
        break;
        //任务栏位于左边
    case PANEL_LEFT:
        x = availableGeo.x() + m_panelSize + MARGIN;
        y = availableGeo.y() + availableGeo.height() - this->height() - MARGIN;
        break;
        //任务栏位于右边
    case PANEL_RIGHT:
        x = availableGeo.x() + availableGeo.width() - m_panelSize - this->width() - MARGIN;
        y = availableGeo.y() + availableGeo.height() - this->height() - MARGIN;
        break;
        //任务栏位于下方
    default:
        if (strcmp(envStr, "ug_CN") == 0 || strcmp(envStr, "kk_KZ") == 0 || strcmp(envStr, "ky_KG") == 0) {
            x = MARGIN;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - MARGIN;
        } else {
            x = availableGeo.x() + availableGeo.width() - this->width() - MARGIN;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - MARGIN;
        }
        break;
    }
    kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x, y, this->width(), this->height()));
    qDebug() << " Position of lingmo-panel is " << m_panelPosition << "; Position of mainwindow is " << this->geometry() << "." << Q_FUNC_INFO << __LINE__;
}

void VpnPage::resetListWidgetWidth()
{
    if (m_listWidget->count() <= MAX_ITEMS) {
        m_listFrame->setFixedWidth(MIN_WIDTH);
    } else {
        m_listFrame->setFixedWidth(MAX_WIDTH);
    }
}

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
#include "windowmanager/windowmanager.h"
#include "lingmostylehelper/lingmostylehelper.h"


VpnPage::VpnPage(QWidget *parent) : SinglePage(parent)
{
    m_activeResourse = new KyActiveConnectResourse(this);
    m_connectResourse = new KyConnectResourse(this);
    m_wiredConnectOperation = new KyWiredConnectOperation(this);

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

    connect(m_wiredConnectOperation, &KyWiredConnectOperation::activateConnectionError, this, &VpnPage::activateFailed);
    connect(m_wiredConnectOperation, &KyWiredConnectOperation::deactivateConnectionError, this, &VpnPage::deactivateFailed);
}

VpnPage::~VpnPage()
{

}

void VpnPage::deleteConnectionMapItem(QMap<QString, QListWidgetItem *> &connectMap,
                             QListWidget *lanListWidget, QString uuid)
{
    QListWidgetItem *p_listWidgetItem = connectMap.value(uuid);
    if (p_listWidgetItem) {
        connectMap.remove(uuid);
        VpnListItem *p_lanItem = (VpnListItem *)lanListWidget->itemWidget(p_listWidgetItem);
        lanListWidget->removeItemWidget(p_listWidgetItem);

        delete p_lanItem;
        p_lanItem = nullptr;

        delete p_listWidgetItem;
        p_listWidgetItem = nullptr;
    }

    return;
}

void VpnPage::clearConnectionMap(QMap<QString, QListWidgetItem *> &connectMap,
                                 QListWidget *lanListWidget)
{
    QMap<QString, QListWidgetItem *>::iterator iter;

    iter = connectMap.begin();
    while (iter != connectMap.end()) {
        qDebug()<<"[VpnPage] clear connection map item"<< iter.key();

        QListWidgetItem *p_widgetItem = iter.value();
        VpnListItem *p_lanItem = (VpnListItem *)lanListWidget->itemWidget(p_widgetItem);
        lanListWidget->removeItemWidget(p_widgetItem);

        delete p_lanItem;
        p_lanItem = nullptr;

        delete p_widgetItem;
        p_widgetItem = nullptr;

        iter = connectMap.erase(iter);
    }

    return;
}

void VpnPage::constructActiveConnectionArea()
{
    QList<KyConnectItem *> activedList;
    QList<KyConnectItem *> netList;
    QGSettings vpnGsettings(GSETTINGS_VPNICON_VISIBLE);

    activedList.clear();
    netList.clear();
    clearConnectionMap(m_activeConnectionMap, m_listWidget);

    m_connectResourse->getVpnAndVirtualConnections(netList);
    KyConnectItem *p_newItem = nullptr;
    if (!netList.isEmpty()) {
        for (int index = 0; index < netList.size(); index++) {
            KyConnectItem *p_netConnectionItem = netList.at(index);
            p_newItem = m_activeResourse->getActiveConnectionByUuid(p_netConnectionItem->m_connectUuid);
            if (p_newItem == nullptr) {
                if (m_netConnectionMap.contains(p_netConnectionItem->m_connectUuid)) {
                    qDebug()<<LOG_FLAG << "has contain uuid" << p_netConnectionItem->m_connectUuid;
                }
                QListWidgetItem *p_listWidgetItem = addNewItem(p_netConnectionItem, m_listWidget);
                m_netConnectionMap.insert(p_netConnectionItem->m_connectUuid, p_listWidgetItem);
            } else {
                if (m_activeConnectionMap.contains(p_netConnectionItem->m_connectUuid)) {
                    qDebug()<<LOG_FLAG << "has contain uuid" << p_netConnectionItem->m_connectUuid;
                }
                QListWidgetItem *p_listWidgetItem = addNewItem(p_newItem, m_listWidget);
                m_activeConnectionMap.insert(p_netConnectionItem->m_connectUuid, p_listWidgetItem);
            }

            delete p_netConnectionItem;
            p_netConnectionItem = nullptr;
        }
        if (vpnGsettings.keys().contains(QString(VISIBLE))) {
            vpnGsettings.set(VISIBLE, true);
        }
    } else {
        if (vpnGsettings.keys().contains(QString(VISIBLE))) {
            vpnGsettings.set(VISIBLE, false);
        }
    }

    if (m_listWidget->count() <= MAX_ITEMS) {
        m_listFrame->setFixedWidth(MIN_WIDTH);
    } else {
        m_listFrame->setFixedWidth(MAX_WIDTH);
    }
}

void VpnPage::initVpnArea()
{
    constructActiveConnectionArea();
}

void VpnPage::resetPageHight()
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

    for (int i = 0; i < m_mainLayout->count(); i ++) {
        QWidget *w = m_mainLayout->itemAt(i)->widget();
        if (w != nullptr && w->isHidden() != true) {
            height += w->height();
        }
    }
    this->setFixedHeight(height + PAGE_SPACE);
}

bool VpnPage::removeConnectionItem(QMap<QString, QListWidgetItem *> &connectMap,
                          QListWidget *lanListWidget, QString path)
{
    QMap<QString, QListWidgetItem *>::iterator iter;
    for (iter = connectMap.begin(); iter != connectMap.end(); ++iter) {
        QListWidgetItem *p_listWidgetItem = iter.value();
        VpnListItem *p_lanItem = (VpnListItem*)lanListWidget->itemWidget(p_listWidgetItem);
        if (p_lanItem->getConnectionPath() == path) {
            qDebug()<<"[VpnPage] Remove a connection from list";

            lanListWidget->removeItemWidget(p_listWidgetItem);

            delete p_lanItem;
            p_lanItem = nullptr;

            delete p_listWidgetItem;
            p_listWidgetItem = nullptr;

            iter = connectMap.erase(iter);
            if (m_listWidget->count() <= MAX_ITEMS) {
                m_listFrame->setFixedWidth(MIN_WIDTH);
            }
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

    removeConnectionItem(m_netConnectionMap, m_listWidget, path);
    removeConnectionItem(m_activeConnectionMap, m_listWidget, path);
    resetPageHight();
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
    if (m_netConnectionMap.contains(p_newItem->m_connectUuid)) {
        qDebug()<<LOG_FLAG << "the connection is exsit" << p_newItem->m_connectUuid;
    }
    m_netConnectionMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);

    delete p_newItem;
    p_newItem = nullptr;
    if (m_listWidget->count() >= MAX_ITEMS) {
        m_listFrame->setFixedWidth(MAX_WIDTH);
    }
    resetPageHight();
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
        VpnListItem *p_lanItem = (VpnListItem *)m_listWidget->itemWidget(p_listWidgetItem);
        QString name1 = p_lanItem->getConnectionName();
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
    VpnListItem *p_lanItem = nullptr;
    if (itemData != nullptr) {
        p_lanItem = new VpnListItem(itemData);
        qDebug() << "[VpnPage] addNewItem, connection: " << itemData->m_connectName;
    } else {
        p_lanItem = new VpnListItem();
        qDebug() << "[VpnPage] Add nullItem!";
    }

    listWidget->setItemWidget(p_listWidgetItem, p_lanItem);
    return p_listWidgetItem;
}

void VpnPage::updateActivatedConnectionArea(KyConnectItem *p_newItem)
{
    if (m_activeConnectionMap.contains(p_newItem->m_connectUuid)) {
        return;
    }

    deleteConnectionMapItem(m_netConnectionMap, m_listWidget, p_newItem->m_connectUuid);
    qDebug()<<"[VpnPage]update active connection item"<<p_newItem->m_connectName;
    deleteConnectionMapItem(m_activeConnectionMap, m_listWidget, p_newItem->m_connectUuid);
    QListWidgetItem *p_listWidgetItem = addNewItem(p_newItem, m_listWidget);
    m_activeConnectionMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);
    if (m_listWidget->count() <= MAX_ITEMS) {
        m_listFrame->setFixedWidth(MIN_WIDTH);
    }

    return;
}

void VpnPage::updateConnectionArea(KyConnectItem *p_newItem)
{
    if (m_netConnectionMap.contains(p_newItem->m_connectUuid)) {
        return;
    }

    deleteConnectionMapItem(m_activeConnectionMap, m_listWidget, p_newItem->m_connectUuid);
    qDebug()<<"[VpnPage] update connection item"<<p_newItem->m_connectName;
    QListWidgetItem *p_listWidgetItem = insertNewItem(p_newItem, m_listWidget);
    m_netConnectionMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);

    if (m_listWidget->count() <= MAX_ITEMS) {
        m_listFrame->setFixedWidth(MIN_WIDTH);
    } else {
        m_listFrame->setFixedWidth(MAX_WIDTH);
    }
}

void VpnPage::updateConnectionState(QMap<QString, QListWidgetItem *> &connectMap,
                                    QListWidget *lanListWidget, QString uuid, ConnectState state)
{
    qDebug() << LOG_FLAG << "update connection state";

    QListWidgetItem *p_listWidgetItem = connectMap.value(uuid);
    if (p_listWidgetItem) {
        VpnListItem *p_lanItem = (VpnListItem *)lanListWidget->itemWidget(p_listWidgetItem);
        p_lanItem->updateConnectionState(state);
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

    if (m_activeConnectionMap.keys().contains(uuid) && state == NetworkManager::ActiveConnection::State::Activated) {
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
        updateConnectionState(m_activeConnectionMap, m_listWidget, uuid, (ConnectState)state);
    } else if (state == NetworkManager::ActiveConnection::State::Deactivated) {
        p_newItem = m_connectResourse->getConnectionItemByUuid(uuid);
        qDebug() << "[VpnPage] deactivated reason" << reason;
        if (nullptr == p_newItem) {
            qWarning()<<"[VpnPage] get active connection failed, connection uuid" << uuid;
            return;
        }

        ssid = p_newItem->m_connectName;
        updateConnectionArea(p_newItem);
        updateConnectionState(m_netConnectionMap, m_listWidget, uuid, (ConnectState)state);
    } else if (state == NetworkManager::ActiveConnection::State::Activating) {
        updateConnectionState(m_netConnectionMap, m_listWidget, uuid, (ConnectState)state);
    } else if (state == NetworkManager::ActiveConnection::State::Deactivating) {
        updateConnectionState(m_activeConnectionMap, m_listWidget, uuid, (ConnectState)state);
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

    if (m_netConnectionMap.contains(newUuid)) {
        QListWidgetItem *p_listWidgetItem = m_netConnectionMap.value(newUuid);
        VpnListItem *p_lanItem = (VpnListItem*)m_listWidget->itemWidget(p_listWidgetItem);
        if (p_connectItem->m_connectName != p_lanItem->getConnectionName()){
            //只要名字改变就要删除，重新插入，主要是为了排序
            deleteConnectionMapItem(m_netConnectionMap, m_listWidget, newUuid);
            QListWidgetItem *p_sortListWidgetItem = insertNewItem(p_connectItem, m_listWidget);
            if (m_netConnectionMap.contains(newUuid)) {
                qDebug()<<LOG_FLAG << "has contained connection" << newUuid;
            }
            m_netConnectionMap.insert(newUuid, p_sortListWidgetItem);
        } else if (p_connectItem->m_connectPath != p_lanItem->getConnectionPath()) {
            p_lanItem->updateConnectionPath(p_connectItem->m_connectPath);
        }
    } else if (!m_activeConnectionMap.contains(newUuid)){
        if (p_connectItem->m_ifaceName.isEmpty()) {
            QListWidgetItem *p_listWidgetItem = insertNewItem(p_connectItem, m_listWidget);
            if (m_netConnectionMap.contains(newUuid)) {
                qDebug()<<LOG_FLAG << "has contained connection uuid" << newUuid;
            }
            m_netConnectionMap.insert(newUuid, p_listWidgetItem);
        }
    } else {
        qWarning() << LOG_FLAG << newUuid <<" is in activemap, so not process.";
    }

    return;
}

void VpnPage::updateActiveConnectionProperty(KyConnectItem *p_connectItem)
{
    QString newUuid = p_connectItem->m_connectUuid;

    if (m_activeConnectionMap.contains(newUuid)) {
        QListWidgetItem *p_listWidgetItem = m_activeConnectionMap.value(newUuid);
        VpnListItem *p_lanItem = (VpnListItem *)m_listWidget->itemWidget(p_listWidgetItem);
        if (p_lanItem->getConnectionName() != p_connectItem->m_connectName) {
            p_lanItem->updateConnectionName(p_connectItem->m_connectName);
        }

        if (p_lanItem->getConnectionName() != p_connectItem->m_connectPath) {
            p_lanItem->updateConnectionPath(p_connectItem->m_connectPath);
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

    if (event->type() == QEvent::ActivationChange) {
        if(QApplication::activeWindow() != this) {
            hide();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void VpnPage::deleteVpn(const QString &connUuid)
{
    qDebug() << "[VpnPage] deleteVpn" << connUuid;
    if (connUuid == nullptr) {
        return;
    }
    m_wiredConnectOperation->deleteWiredConnect(connUuid);
}

void VpnPage::activateVpn(const QString& connUuid)
{
    if (m_netConnectionMap.contains(connUuid)) {
        qDebug() << "[VpnPage] activateVpn" << connUuid;
        m_wiredConnectOperation->activateVpnConnection(connUuid);
    }
}

void VpnPage::deactivateVpn(const QString& connUuid)
{
    qDebug() << "[VpnPage] deactivateVpn" << connUuid;
    QString name("");
    m_wiredConnectOperation->deactivateWiredConnection(name, connUuid);
}

void VpnPage::showDetailPage(QString devName, QString uuid)
{
#ifdef VPN_DETAIL
    KyConnectItem *p_item = nullptr;
    bool isActive = true;

    if (m_connectResourse->isActivatedConnection(uuid)) {
        p_item = m_activeResourse->getActiveConnectionByUuid(uuid);
        isActive = true;
    } else {
        p_item = m_connectResourse->getConnectionItemByUuid(uuid);
        isActive = false;
    }

    if (nullptr == p_item) {
       qWarning()<<"[VpnPage] GetConnectionItemByUuid is empty when showDetailPage."
                      <<"device name"<<devName
                      <<"connect uuid"<<uuid;
       return;
    }

    NetDetail *netDetail = new NetDetail(devName, p_item->m_connectName, uuid, isActive, false, false);
    netDetail->show();

    delete p_item;
    p_item = nullptr;
#endif
}

void VpnPage::showUI()
{
    //2209中窗管在hide界面时会刷新属性，需要重新设置无图标属性
    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
        const KWindowInfo info(this->winId(), NET::WMState);
        if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager)) {
            KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
        }
    }

    kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    resetPageHight();

    showNormal();
    raise();
    activateWindow();
    resetWindowPosition();
    return;
}

void VpnPage::resetWindowPosition()
{

#define MARGIN 4
#define PANEL_TOP 1
#define PANEL_LEFT 2
#define PANEL_RIGHT 3
//#define PANEL_BOTTOM 4
    if (!m_positionInterface) {
        m_positionInterface = new QDBusInterface("org.lingmo.panel",
                            "/panel/position",
                            "org.lingmo.panel",
                            QDBusConnection::sessionBus());
    }
    QRect rect;
    QDBusReply<QVariantList> reply = m_positionInterface->call("GetPrimaryScreenGeometry");
    //reply获取的参数共5个，分别是 主屏可用区域的起点x坐标，主屏可用区域的起点y坐标，主屏可用区域的宽度，主屏可用区域高度，任务栏位置
    if (!m_positionInterface->isValid() || !reply.isValid() || reply.value().size() < 5) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(0, 0, this->width(), this->height()));
        return;
    }
    QVariantList position_list = reply.value();
    int position = position_list.at(4).toInt();
    switch(position){
    case PANEL_TOP:
        //任务栏位于上方
        rect = QRect(position_list.at(0).toInt() + position_list.at(2).toInt() - this->width() - MARGIN,
                     position_list.at(1).toInt() + MARGIN,
                     this->width(), this->height());
        break;
        //任务栏位于左边
    case PANEL_LEFT:
        rect = QRect(position_list.at(0).toInt() + MARGIN,
                     position_list.at(1).toInt() + reply.value().at(3).toInt() - this->height() - MARGIN,
                     this->width(), this->height());
        break;
        //任务栏位于右边
    case PANEL_RIGHT:
        rect = QRect(position_list.at(0).toInt() + position_list.at(2).toInt() - this->width() - MARGIN,
                     position_list.at(1).toInt() + reply.value().at(3).toInt() - this->height() - MARGIN,
                     this->width(), this->height());
        break;
        //任务栏位于下方
    default:
        rect = QRect(position_list.at(0).toInt() + position_list.at(2).toInt() - this->width() - MARGIN,
                     position_list.at(1).toInt() + reply.value().at(3).toInt() - this->height() - MARGIN,
                     this->width(), this->height());
        break;
    }
    kdk::WindowManager::setGeometry(this->windowHandle(), rect);
    qDebug() << " Position of lingmo-panel is " << position << "; Position of mainwindow is " << this->geometry() << "." << Q_FUNC_INFO << __LINE__;
}

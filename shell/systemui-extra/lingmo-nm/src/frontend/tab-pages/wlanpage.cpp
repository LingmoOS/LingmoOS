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
#include "wlanpage.h"
#include "kywirelessnetitem.h"
#include "kwindowsystem.h"
#include "kwindowsystem_export.h"
#include <QEvent>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QScrollBar>

#define AP_SCAN_INTERVAL (20*1000)
#define ICON_REFRESH_INTERVAL (5*1000)

#define LOG_FLAG  "[WlanPage]"
#define LAN_PAGE_INDEX 0

const QString NotApConnection   = "0";
const QString IsApConnection    = "1";

WlanPage::WlanPage(QWidget *parent) : TabPage(parent)
{
    qRegisterMetaType<NetworkManager::Device::State>("NetworkManager::Device::State");
    qRegisterMetaType<NetworkManager::Device::StateChangeReason>("NetworkManager::Device::StateChangeReason");
    m_wirelessNetResource = new KyWirelessNetResource(this);
    m_activatedConnectResource = new KyActiveConnectResourse(this);
    m_netDeviceResource=new KyNetworkDeviceResourse(this);
    m_connectResource = new KyConnectResourse(this);
    m_wirelessConnectOpreation = new KyWirelessConnectOperation(this);

    checkShowWifi6Plus();
    initDevice();
    initWlanUI();
    initWlanSwitchState();
    initDeviceCombox();
    initWlanArea();

    initTimer();

    connect(m_wirelessNetResource, &KyWirelessNetResource::wifiNetworkAdd, this, &WlanPage::onWlanAdded);
    connect(m_wirelessNetResource, &KyWirelessNetResource::wifiNetworkRemove, this, &WlanPage::onWlanRemoved);

    connect(m_wirelessNetResource, &KyWirelessNetResource::signalStrengthChange, this, &WlanPage::signalStrengthChange);
    connect(m_wirelessNetResource, &KyWirelessNetResource::secuTypeChange, this, &WlanPage::onSecurityTypeChange);

    connect(m_wirelessNetResource, &KyWirelessNetResource::connectionAdd, this, &WlanPage::onConnectionAdd);
    connect(m_wirelessNetResource, &KyWirelessNetResource::connectionRemove, this, &WlanPage::onConnectionRemove);
    connect(m_wirelessNetResource, &KyWirelessNetResource::connectionUpdate, this, &WlanPage::onConnectionUpdate);

    connect(m_activatedConnectResource, &KyActiveConnectResourse::stateChangeReason,
                                                    this, &WlanPage::onConnectionStateChanged);

    connect(m_netDeviceResource, &KyNetworkDeviceResourse::deviceAdd, this, &WlanPage::onDeviceAdd);
    connect(m_netDeviceResource, &KyNetworkDeviceResourse::deviceRemove, this, &WlanPage::onDeviceRemove);
    connect(m_netDeviceResource, &KyNetworkDeviceResourse::deviceNameUpdate, this, &WlanPage::onDeviceNameUpdate);

    connect(m_netDeviceResource, &KyNetworkDeviceResourse::deviceManagedChange, this, &WlanPage::onDeviceManagedChanged);
    connect(m_netDeviceResource, &KyNetworkDeviceResourse::stateChanged, this, &WlanPage::onWlanStateChanged);

    connect(m_wirelessConnectOpreation, &KyWirelessConnectOperation::activateConnectionError, this, &WlanPage::activateFailed);
    connect(m_wirelessConnectOpreation, &KyWirelessConnectOperation::addAndActivateConnectionError, this, &WlanPage::activateFailed);
    connect(m_wirelessConnectOpreation, &KyWirelessConnectOperation::deactivateConnectionError, this, &WlanPage::deactivateFailed);

    connect(m_wirelessConnectOpreation, &KyWirelessConnectOperation::wifiEnabledChanged, this, &WlanPage::onWifiEnabledChanged);

    connect(m_connectResource, &KyConnectResourse::connectivityChanged, this, &WlanPage::connectivityChanged);
    connect(m_connectResource, &KyConnectResourse::connectivityCheckSpareUriChanged, this, &WlanPage::connectivityCheckSpareUriChanged);
    connect(m_netSwitch, &KSwitchButton::clicked, this, [=](bool checked) {
        //解决 switchBtn不支持点击的情况下，点击按钮，有无线网卡后不自动开启的问题
        if (getSwitchBtnEnable()) {
            setSwitchBtnState(!checked);
            setWirelessEnable(checked);
        }
    });
    m_wlanPagePtrMap.clear();
    m_joinHiddenWiFiPagePtrMap.clear();
}

bool WlanPage::eventFilter(QObject *w, QEvent *e)
{
    if (w == m_settingsLabel) {
        if (e->type() == QEvent::MouseButtonRelease) {
            //ZJP_TODO 打开控制面板
            qDebug() << LOG_FLAG <<"recive event show control center";
            showControlCenter();
        }
    } else if (w == m_netSwitch) {
        if (e->type() == QEvent::MouseButtonRelease) {
            if (!getSwitchBtnEnable()) {
                showDesktopNotify(tr("No wireless network card detected"), "networkwrong");
                //检测不到无线网卡不再触发click信号
            }
        }
    } else if (w == m_activatedNetListWidget) {
        //去掉无右键菜单显示时的选中效果
        if (e->type() ==  QEvent::FocusIn) {
            if (m_activatedNetListWidget->currentItem() != nullptr) {
                m_activatedNetListWidget->currentItem()->setSelected(false);
            }
        }
    } else if (w == m_inactivatedNetListWidget) {
        //去掉无右键菜单显示时的选中效果
        if (e->type() == QEvent::FocusIn) {
            if (m_inactivatedNetListWidget->currentItem() != nullptr) {
                m_inactivatedNetListWidget->currentItem()->setSelected(false);
            }
        }
    }
    return QWidget::eventFilter(w,e);
}

void WlanPage::initWlanUI()
{
    m_titleLabel->setText(tr("WLAN"));

    m_activatedNetLabel->setText(tr("Activated WLAN"));  
    m_activatedNetListWidget = new QListWidget(m_activatedNetFrame);
    m_activatedNetListWidget->setFrameShape(QFrame::Shape::NoFrame);
    m_activatedNetListWidget->setContentsMargins(MAIN_LAYOUT_MARGINS);
    m_activatedNetListWidget->setSpacing(NET_LIST_SPACING);
    m_activatedNetListWidget->setFixedHeight(NORMAL_HEIGHT);
    m_activatedNetListWidget->setFixedWidth(MIN_WIDTH);
    m_activatedNetListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_activatedNetListWidget->setProperty("needTranslucent", true);
    m_activatedNetLayout->addWidget(m_activatedNetListWidget);

    m_inactivatedNetLabel->setText(tr("Other WLAN"));
    m_inactivatedNetListWidget = new QListWidget(m_inactivatedNetListArea);
    m_inactivatedNetListWidget->setContentsMargins(MAIN_LAYOUT_MARGINS);
    m_inactivatedNetListWidget->setSpacing(NET_LIST_SPACING);
    m_inactivatedNetListWidget->setFrameShape(QFrame::Shape::NoFrame);
    m_inactivatedNetListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inactivatedNetListWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    m_inactivatedNetListWidget->verticalScrollBar()->setProperty("drawScrollBarGroove",false); //去除滚动条的外侧黑框
    m_inactivatedNetListWidget->verticalScrollBar()->setSingleStep(SCROLL_STEP);
    m_inactivatedNetListWidget->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_inactivatedNetListWidget->verticalScrollBar()->setProperty("needTranslucent", true);
    m_inactivatedNetListWidget->setProperty("needTranslucent", true);
    addWlanMoreItem();
    m_inactivatedAreaLayout->addWidget(m_inactivatedNetListWidget);

    connect(m_inactivatedNetListWidget, &QListWidget::currentItemChanged,
            this, &WlanPage::onInactivateListWidgetItemChanged);

    //点击【已连接】网络区域 去掉其他网络的选中效果
    connect(m_activatedNetListWidget, &QListWidget::clicked, this, &WlanPage::setInactivateListItemNoSelect);
    connect(m_activatedNetListWidget, &QListWidget::currentItemChanged, this, &WlanPage::setInactivateListItemNoSelect);

    QPalette pal = m_activatedNetListWidget->palette();
    pal.setBrush(QPalette::Base, QColor(0,0,0,0));       //背景透明
    m_activatedNetListWidget->setPalette(pal);
    m_inactivatedNetListWidget->setPalette(pal);

    m_settingsLabel->installEventFilter(this);
    m_netSwitch->installEventFilter(this);
    m_activatedNetListWidget->installEventFilter(this);
    m_inactivatedNetListWidget->installEventFilter(this);

    showRate();
}

bool WlanPage::getWirelessDevieceUseable()
{
    for (auto devname : m_devList) {
        if (m_netDeviceResource->getDeviceState(devname) >= NetworkManager::Device::Disconnected) {
            return true;
        }
    }
    return false;
}

void WlanPage::setWirelessEnable(bool state)
{
    m_wirelessConnectOpreation->setWirelessEnabled(state);
    return;
}

bool WlanPage::getWirelessEnable()
{
    return m_wirelessConnectOpreation->getWirelessEnabled();
}

void WlanPage::checkShowWifi6Plus()
{
    //990下不显示wifi6+图标
    QProcess * processCpuinfo = new QProcess(this);
    processCpuinfo->start(QString("cat /proc/cpuinfo"));
    connect(processCpuinfo, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, [ = ]() {
        processCpuinfo->deleteLater();
    });
    processCpuinfo->waitForFinished();
    QString ctrCpuinfo = processCpuinfo->readAll();
    if (ctrCpuinfo.indexOf("Kirin", 0, Qt::CaseInsensitive) != -1 && ctrCpuinfo.indexOf("990") != -1) {
        m_showWifi6Plus = false;
    }
}

void WlanPage::initWlanSwitchState()
{
    if (m_devList.isEmpty()) {
        setSwitchBtnState(false);
        setSwitchBtnEnable(false);
        return ;
    }

    setSwitchBtnEnable(true);
    setSwitchBtnState(getWirelessDevieceUseable());
    return;
}

void WlanPage::initTimer()
{
    m_scanTimer = new QTimer(this);
    connect(m_scanTimer, &QTimer::timeout, this, &WlanPage::requestScan);

    m_refreshIconTimer = new QTimer(this);
    connect(m_refreshIconTimer, &QTimer::timeout, this, &WlanPage::onRefreshIconTimer);
    m_refreshIconTimer->start(ICON_REFRESH_INTERVAL);
}

/**
 * @brief WlanPage::initDevice 初始化默认网卡
 */
void WlanPage::initDevice()
{
    m_devList.clear();
    m_netDeviceResource->getNetworkDeviceList(NetworkManager::Device::Type::Wifi, m_devList);

    m_currentDevice = getDefaultDeviceName(WIRELESS);

    if (m_devList.count() == 0) {
        return;
    }
    for (int index = m_devList.count() - 1; index >= 0; --index) {
        if (!m_netDeviceResource->getDeviceManaged(m_devList.at(index))) {
            //删除未托管网卡
            qDebug() << LOG_FLAG << "delete unmanaged device" << m_devList.at(index);
            m_devList.removeOne(m_devList.at(index));
        }
    }
    return;
}

void WlanPage::initDeviceCombox()
{
    //TODO 获取设备列表，单设备时隐藏下拉框，多设备时添加到下拉框
    disconnect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                this, &WlanPage::onDeviceComboxIndexChanged);
    m_deviceComboBox->clear();

    if (getSwitchBtnState()) {
        if (0 == m_devList.count()) {
            m_deviceFrame->show();
            m_tipsLabel->show();
            m_deviceComboBox->hide();
            m_currentDevice = "";
            setDefaultDevice(WIRELESS, m_currentDevice);
        } else if (1 == m_devList.count()) {
            m_deviceFrame->hide();
            if (m_currentDevice != m_devList.at(0)) {
                m_currentDevice = m_devList.at(0);
                setDefaultDevice(WIRELESS, m_currentDevice);
            }
        } else {
            m_deviceFrame->show();
            m_deviceComboBox->show();
            m_tipsLabel->hide();
            for (int index = 0; index < m_devList.count(); ++index) {
                m_deviceComboBox->addItem(m_devList.at(index));
            }

            if (m_devList.contains(m_currentDevice)) {
                m_deviceComboBox->setCurrentText(m_currentDevice);
            } else {
                m_currentDevice = m_deviceComboBox->currentText();
                setDefaultDevice(WIRELESS, m_currentDevice);
            }
        }
    } else {
        m_deviceFrame->hide();
        m_currentDevice = "";
        setDefaultDevice(WIRELESS, m_currentDevice);
    }

    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                            this, &WlanPage::onDeviceComboxIndexChanged, Qt::DirectConnection);
    return;
}

QListWidgetItem *WlanPage::addEmptyItem(QListWidget *wirelessListWidget)
{
    WlanListItem *p_wlanItem = new WlanListItem();
    QListWidgetItem *p_listWidgetItem = new QListWidgetItem();
//    p_listWidgetItem->setFlags(p_listWidgetItem->flags() & (~Qt::ItemIsSelectable));   //设置不可被选中
    p_listWidgetItem->setSizeHint(QSize(wirelessListWidget->width(), p_wlanItem->height()));
    wirelessListWidget->addItem(p_listWidgetItem);
    wirelessListWidget->setItemWidget(p_listWidgetItem, p_wlanItem);

    return p_listWidgetItem;
}

QListWidgetItem *WlanPage::addNewItem(KyWirelessNetItem &wirelessNetItem,
                                      QListWidget *wirelessListWidget)
{
    WlanListItem *p_wlanItem = new WlanListItem(
                wirelessNetItem,
                m_currentDevice,
                m_connectResource->isApConnection(wirelessNetItem.m_connectUuid),
                m_showWifi6Plus);
    connect(p_wlanItem, &WlanListItem::itemHeightChanged, this, &WlanPage::onItemHeightChanged);
    connect(p_wlanItem, &WlanListItem::detailShow, this, &WlanPage::showDetailPage);

    QListWidgetItem *p_listWidgetItem = new QListWidgetItem();
//    p_listWidgetItem->setFlags(p_listWidgetItem->flags() & (~Qt::ItemIsSelectable));
    p_listWidgetItem->setSizeHint(QSize(wirelessListWidget->width(), p_wlanItem->height()));
    wirelessListWidget->addItem(p_listWidgetItem);
    wirelessListWidget->setItemWidget(p_listWidgetItem, p_wlanItem);

    return p_listWidgetItem;
}

QListWidgetItem *WlanPage::insertNewItem(KyWirelessNetItem &wirelessNetItem,
                                         QListWidget *wirelessListWidget,
                                         int row)
{
    WlanListItem *p_wlanItem = new WlanListItem(wirelessNetItem, m_currentDevice, m_showWifi6Plus);
    connect(p_wlanItem, &WlanListItem::itemHeightChanged, this, &WlanPage::onItemHeightChanged);
    connect(p_wlanItem, &WlanListItem::detailShow, this, &WlanPage::showDetailPage);

    QListWidgetItem *p_listWidgetItem = new QListWidgetItem();
//    p_listWidgetItem->setFlags(p_listWidgetItem->flags() & (~Qt::ItemIsSelectable));
    p_listWidgetItem->setSizeHint(QSize(wirelessListWidget->width(), p_wlanItem->height()));
    wirelessListWidget->insertItem(row, p_listWidgetItem);
    wirelessListWidget->setItemWidget(p_listWidgetItem, p_wlanItem);

    return p_listWidgetItem;
}

QListWidgetItem *WlanPage::insertNewItemWithSort(KyWirelessNetItem &wirelessNetItem,
                                         QListWidget *p_ListWidget)
{
    int row = 0;

  //  qDebug()<< "insertNewItemWithSort" << wirelessNetItem.m_NetSsid
  //          <<"sort item config" << wirelessNetItem.m_isConfigured
  //         << "signal strength" << wirelessNetItem.m_signalStrength;
    WlanListItem *p_sortWlanItem = new WlanListItem(wirelessNetItem, m_currentDevice, m_showWifi6Plus);
    connect(p_sortWlanItem, &WlanListItem::itemHeightChanged, this, &WlanPage::onItemHeightChanged);
    connect(p_sortWlanItem, &WlanListItem::detailShow, this, &WlanPage::showDetailPage);

    QListWidgetItem *p_sortListWidgetItem = new QListWidgetItem();
//    p_sortListWidgetItem->setFlags(p_sortListWidgetItem->flags() & (~Qt::ItemIsSelectable));
    p_sortListWidgetItem->setSizeHint(QSize(p_ListWidget->width(), p_sortWlanItem->height()));

   // qDebug() << "insertNewItemWithSort, count" << p_ListWidget->count();
    for (row = 0; row < p_ListWidget->count() - 1; ++row) {
        QListWidgetItem *p_listWidgetItem = p_ListWidget->item(row);
        WlanListItem *p_wlanItem = (WlanListItem *)p_ListWidget->itemWidget(p_listWidgetItem);

        if (WMI_OB_NAME == p_wlanItem->objectName()) {
            qDebug() << "insertNewItemWithSort" << "p_wlanItem is WlanMoreItem";
            continue;
        }

        // qDebug()<< "insertNewItemWithSort" << p_wlanItem->getSsid()
        //         <<"item config" << p_wlanItem->isConfigured()
        //        << "signal strength" << p_wlanItem->getSignalStrength();

        if (wirelessNetItem.m_isConfigured == p_wlanItem->isConfigured()) {
            if (wirelessNetItem.m_signalStrength > p_wlanItem->getSignalStrength()) {
                break;
            }
        } else {
            if (wirelessNetItem.m_isConfigured) {
                break;
            }
        }
    }

    p_ListWidget->insertItem(row, p_sortListWidgetItem);
    p_ListWidget->setItemWidget(p_sortListWidgetItem, p_sortWlanItem);

    //qDebug()<< "insertNewItemWithSort" << "insert sort item finished. row" << row;
    return p_sortListWidgetItem;
}

void WlanPage::clearWirelessNetItemMap(QMap<QString, QListWidgetItem*> &wirelessNetItem,
                                       QListWidget *wirelessListWidget)
{
    QMap<QString, QListWidgetItem*>::iterator iter;

    iter = wirelessNetItem.begin();
    while (iter != wirelessNetItem.end()) {
        QListWidgetItem *p_listWidgetItem = iter.value();
        if (p_listWidgetItem == m_hiddenItem) {
            continue;
        }

        WlanListItem *p_wlanItem = (WlanListItem *)wirelessListWidget->itemWidget(p_listWidgetItem);
        wirelessListWidget->removeItemWidget(p_listWidgetItem);

        delete p_wlanItem;

        delete p_listWidgetItem;

        iter = wirelessNetItem.erase(iter);
    }

    return;
}

void WlanPage::deleteWirelessItemFormMap(QMap<QString, QListWidgetItem*> &wirelessNetItemMap,
                                         QListWidget *wirelessListWidget, QString ssid)
{
    QListWidgetItem *p_listWidgetItem = wirelessNetItemMap.value(ssid);
    if (nullptr == p_listWidgetItem) {
        qWarning()<< LOG_FLAG <<"wireless item is not exsit, it's ssid is " << ssid;
        return;
    }

    WlanListItem *p_wlanItem = (WlanListItem *)wirelessListWidget->itemWidget(p_listWidgetItem);
    if (nullptr == p_wlanItem) {
        qWarning() << LOG_FLAG << "p_wlanItem is null";
        return;
    }

    wirelessNetItemMap.remove(ssid);

    wirelessListWidget->takeItem(wirelessListWidget->row(p_listWidgetItem));

    delete p_wlanItem;

    delete p_listWidgetItem;

    if (m_inactivatedNetListWidget->count() <= MAX_ITEMS) {
        m_inactivatedNetListWidget->setFixedWidth(MIN_WIDTH);
    }
    return;
}

void WlanPage::updateWlanItemState(QListWidget *p_wirelessListWidget,
                                   QListWidgetItem *p_listWidgetItem,
                                   ConnectState state)
{
    WlanListItem *p_wlanItem = nullptr;
    p_wlanItem = (WlanListItem *)p_wirelessListWidget->itemWidget(p_listWidgetItem);
    if (nullptr != p_wlanItem) {
        p_wlanItem->updateConnectState(state);
    }

    return;
}

void WlanPage::constructActivateConnectionArea()
{
    int height = 0;
    clearWirelessNetItemMap(m_activateConnectionItemMap, m_activatedNetListWidget);

    if (!m_currentDevice.isEmpty()) {
        KyWirelessNetItem wirelessNetItem;
        bool ret = m_wirelessNetResource->getActiveWirelessNetItem(m_currentDevice, wirelessNetItem);
        if (ret == true) {
            QListWidgetItem *p_listWidgetItem = addNewItem(wirelessNetItem, m_activatedNetListWidget);
            m_activateConnectionItemMap.insert(wirelessNetItem.m_NetSsid, p_listWidgetItem);
            updateWlanItemState(m_activatedNetListWidget, p_listWidgetItem, Activated);

            height += p_listWidgetItem->sizeHint().height();
            setNetSpeed->start(REFRESH_NETWORKSPEED_TIMER);
            m_activatedNetListWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
        }
    }

    if (height == 0) {  
        QListWidgetItem *p_listWidgetItem = addEmptyItem(m_activatedNetListWidget);
        m_activateConnectionItemMap.insert(EMPTY_SSID, p_listWidgetItem);
        m_activatedNetListWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

        height += p_listWidgetItem->sizeHint().height();
    }

    m_activatedNetListWidget->setFixedHeight(height);

    return;
}

void WlanPage::constructWirelessNetArea()
{
    qDebug() << "[WlanPage] Started loading wireless net list!"
             << QDateTime::currentDateTime().toString("hh:mm:ss.zzzz");

    clearWirelessNetItemMap(m_wirelessNetItemMap, m_inactivatedNetListWidget);
    m_expandedItem = nullptr;

    QList<KyWirelessNetItem> wirelessNetItemList;
    if (!m_wirelessNetResource->getDeviceWifiNetwork(m_currentDevice, wirelessNetItemList)) {
        qWarning()<<"[WlanPage] get wireless net item list failed.";
        return;
    }

    QString activateSsid = m_activateConnectionItemMap.firstKey();

    Q_FOREACH (auto wirelessNetItem, wirelessNetItemList) {
        if (wirelessNetItem.m_NetSsid == activateSsid) {
            continue;
        }

        QListWidgetItem *p_listWidgetItem = addNewItem(wirelessNetItem, m_inactivatedNetListWidget);
        m_wirelessNetItemMap.insert(wirelessNetItem.m_NetSsid, p_listWidgetItem);
        updateWlanItemState(m_inactivatedNetListWidget, p_listWidgetItem, Deactivated);
    }

    addWlanMoreItem();

    qDebug() << "[WlanPage] Stopped loading wireless net list! time="
             << QDateTime::currentDateTime().toString("hh:mm:ss.zzzz");
    if (m_inactivatedNetListWidget->count() <= MAX_ITEMS) {
        m_inactivatedNetListWidget->setFixedWidth(MIN_WIDTH);
    } else {
        m_inactivatedNetListWidget->setFixedWidth(MAX_WIDTH);
    }
    return;
}

void WlanPage::initWlanArea()
{
    if (getSwitchBtnState()) {
        m_activatedNetFrame->show();
        m_activatedNetDivider->show();
        constructActivateConnectionArea();

        m_inactivatedNetFrame->show();
        constructWirelessNetArea();
    } else {
        m_activatedNetFrame->hide();
        m_activatedNetDivider->hide();

        m_inactivatedNetFrame->hide();
    }

    return;
}

void WlanPage::onWlanAdded(QString interface, KyWirelessNetItem &item)
{
    //for dbus
    int category = 0;
    category = item.getCategory(item.m_uni);

    //990 WiFi6+对应图标显示为WiFi6
    if (!m_showWifi6Plus && category == 2) {
        category = 1;
    }

    QStringList info;
    info << item.m_NetSsid
         << QString::number(item.m_signalStrength)
         << item.m_secuType
         << (m_connectResource->isApConnection(item.m_connectUuid) ? IsApConnection : NotApConnection)
         << QString::number(category);
    Q_EMIT wlanAdd(interface, info);

    if (interface != m_currentDevice) {
        qDebug() << "[WlanPage] wlan add interface not equal defaultdevice";
        return;
    }

    if (m_wirelessNetItemMap.contains(item.m_NetSsid)) {
        return;
    }

    qDebug() << "[WlanPage] A Wlan Added! interface = "
             << interface << "; ssid = " << item.m_NetSsid << Q_FUNC_INFO <<__LINE__;

    QListWidgetItem *p_listWidgetItem = insertNewItemWithSort(item, m_inactivatedNetListWidget);
    m_wirelessNetItemMap.insert(item.m_NetSsid, p_listWidgetItem);
    updateWlanItemState(m_inactivatedNetListWidget, p_listWidgetItem, Deactivated);

    addWlanMoreItem();
    if (m_inactivatedNetListWidget->count() > MAX_ITEMS) {
        m_inactivatedNetListWidget->setFixedWidth(MAX_WIDTH);
    }
    return;
}

void WlanPage::onWlanRemoved(QString interface, QString ssid)
{
    Q_EMIT wlanRemove(interface, ssid);

    if (m_wlanPagePtrMap.contains(interface) && m_wlanPagePtrMap[interface].contains(ssid)) {
        if (m_wlanPagePtrMap[interface][ssid] != nullptr) {
            m_wlanPagePtrMap[interface][ssid]->close();
        }
    }

    if (interface != m_currentDevice) {
        qDebug()<<"[WlanPage] the device is not current device,"
                <<"current device" << m_currentDevice
                << "remove wlan device" << interface;
        return;
    }

    if (!m_wirelessNetItemMap.contains(ssid) && !m_activateConnectionItemMap.contains(ssid)) {
        return;
    }

    if (m_expandedItem == m_wirelessNetItemMap.value(ssid)) {
        m_expandedItem = nullptr;
    }

    qDebug() << "[WlanPage] A Wlan Removed! interface = " << interface
             << "; ssid = " << ssid << Q_FUNC_INFO <<__LINE__;


    if (m_wirelessNetItemMap.contains(ssid)) {
        deleteWirelessItemFormMap(m_wirelessNetItemMap,
                                  m_inactivatedNetListWidget, ssid);
    } else {
        deleteWirelessItemFormMap(m_activateConnectionItemMap,
                                  m_activatedNetListWidget, ssid);
//        showDesktopNotify(tr("WLAN Disconnected Successfully"), "networkdisconnected");

        QListWidgetItem *p_listWidgetItem = addEmptyItem(m_activatedNetListWidget);
        m_activateConnectionItemMap.insert(EMPTY_SSID, p_listWidgetItem);
        m_activatedNetListWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    }

    return;
}

void WlanPage::updateWlanListItem(QString ssid)
{
    KyWirelessNetItem wirelessNetItem;

    qDebug() << LOG_FLAG << "update wlan list item " << ssid;

    bool ret = m_wirelessNetResource->getWifiNetwork(m_currentDevice, ssid, wirelessNetItem);
    if (ret) {
        QListWidgetItem *p_listWidgetItem = m_wirelessNetItemMap.value(ssid);
        if (p_listWidgetItem) {
            WlanListItem *p_wlanItem = (WlanListItem *)m_inactivatedNetListWidget->itemWidget(p_listWidgetItem);
            p_wlanItem->updateWirelessNetItem(wirelessNetItem);
            p_wlanItem->updateConnectState(Deactivated);
            Q_EMIT this->wlanConnectChanged(Deactivated);
        }
    }

    return;
}

void WlanPage::onConnectionAdd(QString deviceName, QString ssid)
{
    qDebug() << LOG_FLAG << "one connection is added, it's ssid " << ssid << "device name"<< deviceName;

    if (deviceName == m_currentDevice) {
        updateWlanListItem(ssid);
    }

    return;
}

void WlanPage::onConnectionRemove(QString deviceName, QString ssid, QString path)
{
    qDebug() << LOG_FLAG << "one connection is removed, it's ssid " << ssid << "device name"<< deviceName;
    if (deviceName == m_currentDevice) {
        if (m_activateConnectionItemMap.contains(ssid)) {
            updateWirelessNetArea(nullptr, ssid, deviceName, path);
        }
        updateWlanListItem(ssid);
    }

    return;
}

void WlanPage::onConnectionUpdate(QString deviceName, QString ssid)
{
    if (deviceName == m_currentDevice || deviceName.isEmpty()) {
        updateWlanListItem(ssid);
    }
}

void WlanPage::onSecurityTypeChange(QString devName, QString ssid, QString secuType)
{
    QListWidgetItem *p_listWidgetItem = nullptr;
    WlanListItem *p_wlanItem = nullptr;

    qDebug()<< LOG_FLAG << "security type is chenged";

    if (m_wirelessNetItemMap.contains(ssid)) {
        p_listWidgetItem = m_wirelessNetItemMap.value(ssid);
        p_wlanItem = (WlanListItem*)m_inactivatedNetListWidget->itemWidget(p_listWidgetItem);
    } else if (m_activateConnectionItemMap.contains(ssid)) {
        p_listWidgetItem = m_activateConnectionItemMap.value(ssid);
        p_wlanItem = (WlanListItem*)m_activatedNetListWidget->itemWidget(p_listWidgetItem);
    }

    if (nullptr != p_wlanItem) {
        p_wlanItem->updateWirelessNetSecurity(ssid, secuType);
    }

    Q_EMIT secuTypeChange(devName, ssid, secuType);

    return;
}


void WlanPage::addDeviceToCombox(QString deviceName)
{
    disconnect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                this, &WlanPage::onDeviceComboxIndexChanged);
    if (getSwitchBtnState()) {
        if (m_currentDevice.isEmpty()){
            m_deviceFrame->hide();
            m_currentDevice = deviceName;
            setDefaultDevice(WIRELESS, m_currentDevice);
        } else if (m_deviceComboBox->count() == 0 && m_currentDevice != deviceName) {
            m_deviceComboBox->addItem(m_currentDevice);
            m_deviceComboBox->addItem(deviceName);
            m_deviceFrame->show();
            m_deviceComboBox->show();
            m_tipsLabel->hide();
        } else {
            m_deviceComboBox->addItem(deviceName);
        }
    }

    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                            this, &WlanPage::onDeviceComboxIndexChanged);
    return;
}

void WlanPage::onDeviceAdd(QString deviceName, NetworkManager::Device::Type deviceType)
{
    qDebug() << LOG_FLAG << "currentDevice" << m_currentDevice << "deviceAdd" << deviceName;
    if (deviceType !=  NetworkManager::Device::Type::Wifi) {
        return;
    }
    
    if (m_devList.contains(deviceName)) {
        return;
    }

    if (!m_netDeviceResource->getDeviceManaged(deviceName)) {
        qWarning() << LOG_FLAG << "couldn not add device" << deviceName << ", it is unmanaged";
        return;
    }

    m_devList << deviceName;
    setSwitchBtnEnable(true);
    setSwitchBtnState(getWirelessDevieceUseable());

    addDeviceToCombox(deviceName);
    if (m_currentDevice == deviceName) {
        initWlanArea();
    }

    Q_EMIT wirelessDeviceStatusChanged();

    return;
}

void WlanPage::deleteDeviceFromCombox(QString deviceName)
{
    disconnect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                this, &WlanPage::onDeviceComboxIndexChanged);

    if (0 == m_devList.count()) {
        m_deviceFrame->hide();
        //m_tipsLabel->show();
        //m_deviceComboBox->hide();
        m_currentDevice = "";
        setDefaultDevice(WIRELESS, m_currentDevice);
    } else if (1 == m_devList.count()) {
        m_deviceFrame->hide();
        m_deviceComboBox->clear();
        m_currentDevice = m_devList.at(0);
        setDefaultDevice(WIRELESS, m_currentDevice);
    } else {
        int index = m_deviceComboBox->findText(deviceName);
        if (-1 != index) {
            m_deviceComboBox->removeItem(index);
            m_currentDevice = m_deviceComboBox->currentText();
            setDefaultDevice(WIRELESS, m_currentDevice);
        }
    }

    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                            this, &WlanPage::onDeviceComboxIndexChanged);
    return;
}

void WlanPage::onDeviceRemove(QString deviceName)
{
    if (!m_devList.contains(deviceName)) {
        return;
    }

    qDebug() << "deviceRemove" << deviceName;

    QString originalDeviceName = m_currentDevice;

    if (m_joinHiddenWiFiPagePtrMap.contains(deviceName)) {
        if (m_joinHiddenWiFiPagePtrMap[deviceName] != nullptr) {
            m_joinHiddenWiFiPagePtrMap[deviceName]->close();
        }
    }

    m_devList.removeOne(deviceName);
    deleteDeviceFromCombox(deviceName);

    if (m_devList.isEmpty()) {
        setSwitchBtnState(false);
        setSwitchBtnEnable(false);
    }

    if (originalDeviceName == deviceName) {
        initWlanArea();
    }

    Q_EMIT wirelessDeviceStatusChanged();
}

void WlanPage::updateDeviceForCombox(QString oldDeviceName, QString newDeviceName)
{
    if (m_currentDevice == oldDeviceName) {
        m_currentDevice = newDeviceName;
        setDefaultDevice(WIRELESS, m_currentDevice);
    }

    int index = m_deviceComboBox->findText(oldDeviceName);
    if (index != -1) {
        m_deviceComboBox->setItemText(index, newDeviceName);
    }

    return;
}

void WlanPage::onDeviceNameUpdate(QString oldName, QString newName)
{
    if (!m_devList.contains(oldName)) {
        return;
    }

    qDebug()<<"[WlanPage] update device name, old name"
           << oldName << "new Name" << newName;

    m_devList.removeOne(oldName);
    m_devList.append(newName);

    QString originalDeviceName = m_currentDevice;

    updateDeviceForCombox(oldName, newName);
    if (originalDeviceName != m_currentDevice) {
        initWlanArea();
    }

    Q_EMIT deviceNameChanged(oldName, newName, WIRELESS);
}

void WlanPage::onWlanStateChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason)
{
    if (getSwitchBtnState() == getWirelessDevieceUseable()) {
        return ;
    }
    setSwitchBtnEnable(true);
    setSwitchBtnState(getWirelessDevieceUseable());
    initDeviceCombox();
    initWlanArea();
    Q_EMIT wirelessSwitchBtnChanged(getSwitchBtnState());
}

void WlanPage::onDeviceManagedChanged(QString deviceName, bool managed)
{
    if (!m_netDeviceResource->checkDeviceType(deviceName, NetworkManager::Device::Type::Wifi)) {
        return;
    }

    if (managed && !m_devList.contains(deviceName)) {
        //添加新增托管网卡
        onDeviceAdd(deviceName, NetworkManager::Device::Type::Wifi);
        qDebug() << LOG_FLAG << "add managed device" << deviceName;
    }
    if (!managed && m_devList.contains(deviceName)) {
        //删除未托管网卡
        onDeviceRemove(deviceName);
        qDebug() << LOG_FLAG << "delete unmanaged device" << deviceName;
    }
}

void WlanPage::onInactivateListWidgetItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous != nullptr && previous->sizeHint().height() > NORMAL_HEIGHT) {

        QSize normalSize(previous->sizeHint().width(), NORMAL_HEIGHT);
        previous->setSizeHint(normalSize);
        WlanListItem *p_wlanItem = (WlanListItem*)m_inactivatedNetListWidget->itemWidget(previous);
        if (p_wlanItem != nullptr) {
            p_wlanItem->setExpanded(false);
        }
    }

    if (current != nullptr) {
        current->setSelected(false);
    }
}

void WlanPage::setInactivateListItemNoSelect()
{
    if (m_inactivatedNetListWidget->currentItem() != nullptr
            && m_inactivatedNetListWidget->currentItem()->sizeHint().height() > NORMAL_HEIGHT) {

        WlanListItem *p_wlanItem = (WlanListItem*)m_inactivatedNetListWidget->itemWidget(m_inactivatedNetListWidget->currentItem());
        if (p_wlanItem != nullptr) {
            p_wlanItem->setExpanded(false);
        }
        m_inactivatedNetListWidget->currentItem()->setSelected(false);
    }
}

void WlanPage::sendApStateChangeSignal(QString uuid,
                                       QString ssid,
                                       QString deviceName,
                                       NetworkManager::ActiveConnection::State state)
{
    bool ret = false;
    ret = m_connectResource->isApConnection(uuid);
    if (!ret) {
        return;
    }

    if (state == NetworkManager::ActiveConnection::State::Deactivated) {
        qDebug() << "[WlanPage] hotspot Deactivated";
        Q_EMIT hotspotDeactivated(deviceName, ssid);
    } else if (state == NetworkManager::ActiveConnection::State::Activated) {
        QString activePath;
        QString settingPath;
        activePath = m_activatedConnectResource->getAcitveConnectionPathByUuid(uuid);
        settingPath = m_connectResource->getApConnectionPathByUuid(uuid);
        qDebug() << "[WlanPage] hotspot activated"<<deviceName<<ssid<<uuid<<activePath<<settingPath;
        Q_EMIT hotspotActivated(deviceName, ssid, uuid, activePath, settingPath);
    }

    return;
}

#if 0
void WlanPage::wlanShowNotify(QString ssid, NetworkManager::ActiveConnection::State state,
                              NetworkManager::ActiveConnection::Reason reason)
{
    if (state == NetworkManager::ActiveConnection::State::Deactivated) {
        showDesktopNotify(tr("WLAN Disconnected Successfully"), "networkdisconnected");
        qDebug() << "[WlanPage] wlan disconnected successfully"  << Q_FUNC_INFO <<__LINE__;
    } else if (state == NetworkManager::ActiveConnection::State::Activated) {
        qDebug() << "[WlanPage] wlan connected successfully" << Q_FUNC_INFO <<__LINE__;
        this->showDesktopNotify(tr("WLAN Connected Successfully"), "networkconnected");
    }

    return;
}
#endif

void WlanPage::updateActivatedArea(QString uuid, QString ssid, QString devName)
{
    m_activatedNetListWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    if (m_activateConnectionItemMap.contains(ssid)) {
        return;
    }

    if (m_expandedItem == m_wirelessNetItemMap.value(ssid)) {
        m_expandedItem = nullptr;
    }
    deleteWirelessItemFormMap(m_wirelessNetItemMap,
                                      m_inactivatedNetListWidget, ssid);

    KyWirelessNetItem wirelessNetItem;
    bool ret = m_wirelessNetResource->getActiveWirelessNetItem(devName, wirelessNetItem);
    if (!ret) {
        qWarning()<<"[WlanPage] get wireless item failed, when update activated connection area.";
        return;
    }

    deleteWirelessItemFormMap(m_activateConnectionItemMap, m_activatedNetListWidget, EMPTY_SSID);
    QListWidgetItem *p_listWidgetItem = addNewItem(wirelessNetItem, m_activatedNetListWidget);
    m_activateConnectionItemMap.insert(wirelessNetItem.m_NetSsid, p_listWidgetItem);

    m_activatedNetListWidget->setFixedHeight(p_listWidgetItem->sizeHint().height());
    if (m_inactivatedNetListWidget->count() <= MAX_ITEMS) {
        m_inactivatedNetListWidget->setFixedWidth(MIN_WIDTH);
    }
    return;
}

void WlanPage::updateWirelessNetArea(QString uuid, QString ssid, QString devName, QString path)
{
    if(m_wirelessNetItemMap.contains(ssid)) {
        return;
    }

    if (m_activateConnectionItemMap.contains(ssid)) {
        QListWidgetItem *p_listWidgetItem = m_activateConnectionItemMap.value(ssid);
        if (nullptr == p_listWidgetItem) {
            qWarning()<< LOG_FLAG <<"wireless item is not exsit, it's ssid is " << ssid;
            return;
        }
        WlanListItem *p_wlanItem = (WlanListItem *)m_activatedNetListWidget->itemWidget(p_listWidgetItem);
        if (nullptr == p_wlanItem) {
            qWarning() << LOG_FLAG << "p_wlanItem is null";
            return;
        }

        if (p_wlanItem->getUuid() == uuid || uuid.isEmpty()) {
            if (uuid.isEmpty() && p_wlanItem->getPath() != path) {
                return;
            }

            deleteWirelessItemFormMap(m_activateConnectionItemMap, m_activatedNetListWidget, ssid);
            QListWidgetItem *p_activeListWidgetItem = addEmptyItem(m_activatedNetListWidget);
            m_activateConnectionItemMap.insert(EMPTY_SSID, p_activeListWidgetItem);
            m_activatedNetListWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
            m_activatedNetListWidget->setFixedHeight(p_activeListWidgetItem->sizeHint().height());
        }
    } else {
        qDebug() << LOG_FLAG << ssid << "is not in activeconnection map";
    }

    KyWirelessNetItem wirelessNetItem;
    bool ret = m_wirelessNetResource->getWifiNetwork(devName, ssid, wirelessNetItem);
    if (!ret) {
        qWarning()<<"[WlanPage] get wireless item failed, when update wireless area.";
        return;
    }

    QListWidgetItem *p_listWidgetItem = insertNewItemWithSort(wirelessNetItem, m_inactivatedNetListWidget);
    m_wirelessNetItemMap.insert(wirelessNetItem.m_NetSsid, p_listWidgetItem);

    // 更新‘更多’条目，以保证其处于listwidget的最底部
    addWlanMoreItem();
    if (m_inactivatedNetListWidget->count() <= MAX_ITEMS) {
        m_inactivatedNetListWidget->setFixedWidth(MIN_WIDTH);
    } else {
        m_inactivatedNetListWidget->setFixedWidth(MAX_WIDTH);
    }
    return;
}

void WlanPage::onConnectionStateChanged(QString uuid,
                                      NetworkManager::ActiveConnection::State state,
                                      NetworkManager::ActiveConnection::Reason reason)
{
    QString devName, ssid;
    m_wirelessNetResource->getSsidByUuid(uuid, ssid);
    m_wirelessNetResource->getDeviceByUuid(uuid, devName);

    qDebug()<< LOG_FLAG << "Q_EMIT wlanActiveConnectionStateChanged" << devName << ssid << state;
    Q_EMIT wlanActiveConnectionStateChanged(devName, ssid, uuid, state);

    if (!m_connectResource->isWirelessConnection(uuid)) {
         qDebug()<< LOG_FLAG << "it is not wireless connection" << uuid;
        return;
    }

    Q_EMIT this->wlanConnectChanged(state);

    bool isApConnection = m_connectResource->isApConnection(uuid);

    if (isApConnection) {
        sendApStateChangeSignal(uuid, ssid, devName, state);
    } else {
        if (state == NetworkManager::ActiveConnection::State::Deactivated &&
                !m_activateConnectionItemMap.contains(ssid)) {
            qDebug() << "wlan remove before deactivated";
        } else {
//            wlanShowNotify(ssid, state, reason);
        }
    }

    if (!devName.isEmpty() && devName != m_currentDevice) {
        return;
    }

    if (state == NetworkManager::ActiveConnection::State::Deactivating ||
            state == NetworkManager::ActiveConnection::State::Deactivated) {
        if (!m_activateConnectionItemMap.contains(ssid)) {
            //check uuid
            QMap<QString, QListWidgetItem*>::iterator iter;
            for(iter = m_activateConnectionItemMap.begin(); iter!= m_activateConnectionItemMap.end(); iter++)
            {
                if(iter.value() != nullptr) {
                    QListWidgetItem *p_listWidgetItem = iter.value();
                    WlanListItem *p_wlanItem = (WlanListItem*)m_activatedNetListWidget->itemWidget(p_listWidgetItem);
                    if (p_wlanItem->getUuid() == uuid) {
                        ssid = p_wlanItem->getSsid();
                        break;
                    }
                }
            }
        }
    }

    qDebug()<< "[WlanPage] wlan state changed, ssid = " << ssid
            << "; state = " << state << "; reason = " << reason << Q_FUNC_INFO <<__LINE__;
    if (state == NetworkManager::ActiveConnection::State::Activated) {
        m_updateStrength = true;
        updateActivatedArea(uuid, ssid, devName);
        setNetSpeed->start(REFRESH_NETWORKSPEED_TIMER);
        if (m_activateConnectionItemMap.contains(ssid)) {
            QListWidgetItem *p_listWidgetItem = m_activateConnectionItemMap.value(ssid);
            updateWlanItemState(m_activatedNetListWidget, p_listWidgetItem, Activated);
        }
    } else if (state == NetworkManager::ActiveConnection::State::Deactivated) {
        m_updateStrength = true;
        if (devName.isEmpty()) {
            devName = m_currentDevice;
        }

        updateWirelessNetArea(uuid, ssid, devName,"");
        setNetSpeed->stop();

        if (m_wirelessNetItemMap.contains(ssid)) {
            QListWidgetItem *p_listWidgetItem = m_wirelessNetItemMap.value(ssid);
            updateWlanItemState(m_inactivatedNetListWidget, p_listWidgetItem, Deactivated);
        }
    } else if (state == NetworkManager::ActiveConnection::State::Deactivating){
        m_updateStrength = false;
        if (m_activateConnectionItemMap.contains(ssid)) {
            QListWidgetItem *p_listWidgetItem = m_activateConnectionItemMap.value(ssid);
            updateWlanItemState(m_activatedNetListWidget, p_listWidgetItem, Deactivating);
        }
    } else if (state == NetworkManager::ActiveConnection::State::Activating) {
        m_updateStrength = false;
        if (m_wirelessNetItemMap.contains(ssid)) {
            QListWidgetItem *p_listWidgetItem = m_wirelessNetItemMap.value(ssid);
            updateWlanItemState(m_inactivatedNetListWidget, p_listWidgetItem, Activating);
        }
    }
    return;
}

void WlanPage::onItemHeightChanged(const bool isExpanded, const QString &ssid)
{
    if (!m_wirelessNetItemMap.contains(ssid)) {
        return;
    }

    QListWidgetItem *p_listWidgetItem = m_wirelessNetItemMap.value(ssid);

    qDebug()<<LOG_FLAG << "expanded changed" << isExpanded << ssid;

    int height = p_listWidgetItem->sizeHint().height();
    int width = p_listWidgetItem->sizeHint().width();

    if (isExpanded) {
        if (m_expandedItem != p_listWidgetItem) {
            onInactivateListWidgetItemChanged(p_listWidgetItem, m_expandedItem);
            qDebug()<<LOG_FLAG << "expanded wlan item";
            m_expandedItem = p_listWidgetItem;
            QSize expandedSize(width, EXPANDED_HEIGHT);
            p_listWidgetItem->setSizeHint(expandedSize);
        }
        m_inactivatedNetListWidget->scrollToItem(p_listWidgetItem, QAbstractItemView::QAbstractItemView::EnsureVisible);
    } else {
        m_expandedItem = nullptr;

        if (height > NORMAL_HEIGHT) {
            qDebug()<<LOG_FLAG<<"do not expanded";
            QSize normalSize(width, NORMAL_HEIGHT);
            p_listWidgetItem->setSizeHint(normalSize);
        }
    }

    return;
}

void WlanPage::onDeviceComboxIndexChanged(int currentIndex)
{
    qDebug() << "onDeviceComboxIndexChanged";
    if (!m_deviceComboBox || currentIndex < 0) {
        return;
    }

    m_currentDevice = m_deviceComboBox->itemText(currentIndex);
    setDefaultDevice(WIRELESS, m_currentDevice);

    initWlanArea();

    Q_EMIT timeToUpdate();

    return;
}

//申请触发扫描，初始化执行&定时执行
void WlanPage::requestScan()
{
    if (!m_wirelessConnectOpreation) {
        qWarning() << "[WlanPage]Scan failed! m_wirelessConnectOpreation is nullptr!" << Q_FUNC_INFO << __LINE__;
        return;
    }

    m_wirelessConnectOpreation->requestWirelessScan();

    return;
}

void WlanPage::onHiddenWlanClicked()
{
    qDebug() << "[wlanPage] AddHideWifi Clicked! " << Q_FUNC_INFO << __LINE__ ;
    showHiddenWlanPage(m_currentDevice);
}

void WlanPage::showControlCenter()
{
    qDebug() << LOG_FLAG <<"show control center";

    QProcess process;
    process.startDetached("lingmo-control-center -m wlanconnect");

    return;
}

void WlanPage::onWifiEnabledChanged(bool isWifiOn)
{
    //监听外部命令导致wifi状态变化，更新界面
    qDebug() << "[WlanPage] onWifiEnabledChanged wifi state" << isWifiOn;
    return;
}

void WlanPage::refreshActiveConnectionIcon(QString ssid, const int &signal)
{
    QListWidgetItem *p_listWidgetItem = m_activateConnectionItemMap.value(ssid);
    if (p_listWidgetItem) {
        WlanListItem *p_wlanItem = (WlanListItem *)m_activatedNetListWidget->itemWidget(p_listWidgetItem);
        if (nullptr != p_wlanItem) {
            p_wlanItem->setSignalStrength(signal);
            return;
        }
    }
}

void WlanPage::onRefreshIconTimer()
{  
    if (!m_updateStrength) {
        return;
    }

    Q_EMIT timeToUpdate();

    if(!this->isVisible()) {
        return;
    }
    //qDebug() << "onRefreshIconTimer";

    if (m_expandedItem) {
        qDebug()<< LOG_FLAG << "Has expanded item and forbid refresh wifi strength" << Q_FUNC_INFO << __LINE__;
        return;
    }

    QList<KyWirelessNetItem> wlanList;
    if (!m_wirelessNetResource->getDeviceWifiNetwork(m_currentDevice, wlanList)) {
        return;
    }

    //解决没有无线设备且m_currentDevice不清空的情况下，m_activateConnectionItemMap为空造成的段错误
    if (m_activateConnectionItemMap.isEmpty()) {
        return ;
    }
    QString activateSsid = m_activateConnectionItemMap.firstKey();
    int sortRow = 0;
    for (int index = 0; index < wlanList.length(); index++) {
        KyWirelessNetItem sortItem = wlanList.at(index);
        QString sortSsid = sortItem.m_NetSsid; //应该在第currentRow行的新的WiFi名称
        //qDebug()<< LOG_FLAG << "sort ssid"<< sortSsid << "active ssid" << activateSsid << "sort row"<< sortRow;
        if (sortSsid == activateSsid) { //排除已连接WiFi
            int signalStrength = 0;
            QString uni, secuType;
            m_netDeviceResource->getActiveConnectionInfo(m_currentDevice, signalStrength, uni, secuType);
            refreshActiveConnectionIcon(activateSsid, signalStrength);
            continue;
        }

        QListWidgetItem *p_listWidgetItem = m_inactivatedNetListWidget->item(sortRow);
        if (p_listWidgetItem) {
            WlanListItem *p_wlanItem = (WlanListItem *)m_inactivatedNetListWidget->itemWidget(p_listWidgetItem);

            if (nullptr == p_wlanItem) {
                qDebug() << LOG_FLAG << "p_wlanItem is null continue";
                continue;//暂时先保持continue，后续讨论是否使用break直接跳出循环
            }

            // 该item是‘更多’条目，不需要更新
            if (WMI_OB_NAME == p_wlanItem->objectName()) {
                qDebug() << LOG_FLAG << "p_wlanItem is WlanMoreItem";
                continue;
            }

            //qDebug()<< LOG_FLAG << "row" << sortRow << "item ssid" << p_wlanItem->getSsid();
            if (sortSsid == p_wlanItem->getSsid()) {
               // qDebug()<< LOG_FLAG << "sort wlan set signal strength." << Q_FUNC_INFO << __LINE__;
                p_wlanItem->setSignalStrength(sortItem.m_signalStrength);
                sortRow++;
                continue;
            }

            QListWidgetItem *p_sortListWidgetItem = m_wirelessNetItemMap.value(sortSsid);
            if (p_sortListWidgetItem) {
                WlanListItem *p_wlanItem = (WlanListItem *)m_inactivatedNetListWidget->itemWidget(p_listWidgetItem);
                if (p_wlanItem == nullptr) {
                    continue;
                }

                if (Deactivated != p_wlanItem->getConnectionState()) {
                    continue;
                }
               // qDebug()<< LOG_FLAG << "sort wlan update position." << sortSsid << Q_FUNC_INFO << __LINE__;
                int takeRow = m_inactivatedNetListWidget->row(p_sortListWidgetItem);
                m_inactivatedNetListWidget->takeItem(takeRow);

                WlanListItem *p_sortWlanItem = new WlanListItem(sortItem, m_currentDevice, m_showWifi6Plus);
                connect(p_sortWlanItem, &WlanListItem::itemHeightChanged, this, &WlanPage::onItemHeightChanged);
                connect(p_sortWlanItem, &WlanListItem::detailShow, this, &WlanPage::showDetailPage);
                m_inactivatedNetListWidget->insertItem(sortRow, p_sortListWidgetItem);
                m_inactivatedNetListWidget->setItemWidget(p_sortListWidgetItem, p_sortWlanItem);
                updateWlanItemState(m_inactivatedNetListWidget, p_sortListWidgetItem, Deactivated);
                sortRow++;
                continue;
            }
        }

        //qDebug()<< LOG_FLAG << "sort wlan add new item." << Q_FUNC_INFO << __LINE__;
        QListWidgetItem *p_newListWidgetItem = insertNewItem(sortItem, m_inactivatedNetListWidget, sortRow);
        m_wirelessNetItemMap.insert(sortItem.m_NetSsid, p_newListWidgetItem);
        updateWlanItemState(m_inactivatedNetListWidget, p_listWidgetItem, Deactivated);

        sortRow++;
    }

   // qDebug()<< LOG_FLAG << "sort wlan finished." << Q_FUNC_INFO << __LINE__;

    return;
}

//for dbus
void WlanPage::getWirelessList(QString devName, QList<QStringList> &list)
{
    KyWirelessNetItem data;

    QList<KyWirelessNetItem> wlanList;
    if (!m_wirelessNetResource->getDeviceWifiNetwork(devName, wlanList)) {
        return;
    }

    QString activeSsid ;
    //先是已连接
    if (m_wirelessNetResource->getActiveWirelessNetItem(devName, data)) {
        qDebug() << "find " << devName;
        QString ssid ="";
        m_wirelessNetResource->getSsidByUuid(data.m_connectUuid, ssid);
        if (m_wirelessNetResource->getWifiNetwork(devName, ssid, data)) {
            int category = 0;
            int signalStrength;
            QString uni,secuType;

            if (m_netDeviceResource->getActiveConnectionInfo(devName, signalStrength, uni, secuType)) {
                category = data.getCategory(uni);
            }
            if (!m_showWifi6Plus && category == 2) {
                category = 1;
            }
            list.append(QStringList() << data.m_NetSsid
                          << QString::number(signalStrength)
                          << secuType
                          << data.m_connectUuid
                          << (m_connectResource->isApConnection(data.m_connectUuid) ? IsApConnection : NotApConnection)
                          << QString::number(category));
            activeSsid = data.m_NetSsid;
        } else {
            list.append(QStringList("--"));
        }
    } else {
        list.append(QStringList("--"));
    }
    //未连接
    Q_FOREACH (auto itemData, wlanList) {
        if (itemData.m_NetSsid == activeSsid) {
            continue;
        }
        int category = 0;
        category = itemData.getCategory(itemData.m_uni);
        if (!m_showWifi6Plus && category == 2) {
            category = 1;
        }
        list.append(QStringList()<<itemData.m_NetSsid
                      << QString::number(itemData.m_signalStrength)
                      << itemData.m_secuType
                      << (m_connectResource->isApConnection(itemData.m_connectUuid) ? IsApConnection : NotApConnection)
                      << QString::number(category));
    }
}

//for dbus
bool WlanPage::getWirelessSwitchBtnState()
{
    return getSwitchBtnState();
}

//开启热点
void WlanPage::activeWirelessAp(const QString apName, const QString apPassword, const QString wirelessBand, const QString apDevice)
{
    QString uuid("");
    QList<KyApConnectItem *> apConnectItemList;
    m_connectResource->getApConnections(apConnectItemList);
    if (!apConnectItemList.isEmpty()) {
        uuid = apConnectItemList.at(0)->m_connectUuid;
    }

    m_wirelessConnectOpreation->activeWirelessAp(uuid, apName, apPassword, apDevice, wirelessBand);
}

//断开热点
void WlanPage::deactiveWirelessAp(const QString apName, const QString uuid)
{
    if (!uuid.isEmpty()) {
        m_wirelessConnectOpreation->deactiveWirelessAp(apName, uuid);
    }
}

void WlanPage::getStoredApInfo(QStringList &list)
{
    list.clear();
    QList<KyApConnectItem *> apConnectItemList;
    m_connectResource->getApConnections(apConnectItemList);
    if (!apConnectItemList.isEmpty()) {
        list << apConnectItemList.at(0)->m_connectSsid;
        list << apConnectItemList.at(0)->m_password;
        list << apConnectItemList.at(0)->m_ifaceName;
        list << (apConnectItemList.at(0)->m_isActivated? "true":"false");
        list << apConnectItemList.at(0)->m_connectUuid;
        list << apConnectItemList.at(0)->m_band;
    }
}

void WlanPage::getApConnectionPath(QString &path, QString uuid)
{
    path.clear();
    path = m_connectResource->getApConnectionPathByUuid(uuid);
}

void WlanPage::getActiveConnectionPath(QString &path, QString uuid)
{
    path.clear();
    path = m_activatedConnectResource->getAcitveConnectionPathByUuid(uuid);
}

void WlanPage::getApInfoBySsid(QString devName, QString ssid, QStringList &list)
{
    list.clear();
    QList<KyApConnectItem *> apConnectItemList;
    m_connectResource->getApConnections(apConnectItemList);
    for (int i = 0; i < apConnectItemList.size(); i++) {
        if (apConnectItemList.at(i)->m_connectSsid == ssid
                && apConnectItemList.at(i)->m_ifaceName == devName) {
            list << apConnectItemList.at(i)->m_password;
            list << apConnectItemList.at(i)->m_band;
        }
    }

    return;
}

void WlanPage::activateWirelessConnection(const QString& devName, const QString& ssid)
{
    KyWirelessNetItem wirelessNetItem;
    if (!m_wirelessNetResource->getWifiNetwork(devName, ssid, wirelessNetItem)) {
        qDebug() << "[WlanPage] no such wifi " << ssid << " in " << devName;
        return;
    }

    if (devName != m_currentDevice) {
        int index = m_deviceComboBox->findText(devName);
        if (index >= 0) {
            m_deviceComboBox->setCurrentIndex(index);
        } else {
            qDebug() << "[WlanPage]activateWirelessConnection no such " << devName;
            return;
        }
    }

    QListWidgetItem *p_listWidgetItem = nullptr;
    WlanListItem *p_wlanItem = nullptr;

    if (m_wirelessNetItemMap.contains(ssid)) {
        Q_EMIT showMainWindow(WLAN_PAGE_INDEX);
        p_listWidgetItem = m_wirelessNetItemMap.value(ssid);
        p_wlanItem = (WlanListItem*)m_inactivatedNetListWidget->itemWidget(p_listWidgetItem);

        m_inactivatedNetListWidget->scrollToItem(p_listWidgetItem, QAbstractItemView::EnsureVisible);


        QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(0,0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::postEvent(p_wlanItem, event);
    } else {
        qDebug() << "[WlanPage]activateWirelessConnection no such " << ssid << "in" << devName;
    }
    return;
}

void WlanPage::deactivateWirelessConnection(const QString& devName, const QString& ssid)
{
    KyWirelessNetItem wirelessNetItem;
    if (!m_wirelessNetResource->getWifiNetwork(devName, ssid, wirelessNetItem)) {
        qDebug() << "[WlanPage] no such wifi " << ssid << " in " << devName;
        return;
    }

    if (!m_connectResource->isActivatedConnection(wirelessNetItem.m_connectUuid)) {
        qDebug()<<"[WlanPage]"<<wirelessNetItem.m_connName << "is not activated, so not deactivated.";
        return;
    }

    qDebug() << "[WlanPage] deActivate Wireless Connection" << devName << ssid;

    m_wirelessConnectOpreation->deActivateWirelessConnection(wirelessNetItem.m_connName, wirelessNetItem.m_connectUuid);

    return;
}

void WlanPage::onMainWindowVisibleChanged(const bool &visible)
{
    qDebug() << "[WlanPage] Received signal of mainwindow visible changed. cur_state = " << visible << Q_FUNC_INFO << __LINE__;
    //开关关闭状态不刷新
    if (visible && getSwitchBtnState()) {
        //打开页面时先触发一次扫描，然后定时扫描wifi热点和刷新icon
        requestScan();
        m_scanTimer->start(AP_SCAN_INTERVAL);
        onRefreshIconTimer();
//        m_refreshIconTimer->start(ICON_REFRESH_INTERVAL);
    } else {
        //界面关闭的时候，停止wifi扫描和刷新
        m_scanTimer->stop();
//        m_refreshIconTimer->stop();
        qDebug() << "wlanpage not visible";
        showNonePwd();
    }

    return;
}

void WlanPage::onWlanPageVisibleChanged(int index)
{
    if (index == LAN_PAGE_INDEX) {
        qDebug() << "wlanpage not visible";
        showNonePwd();
    }

    m_inactivatedNetListWidget->setCurrentIndex(QModelIndex());  // 去除item选中效果
}

void WlanPage::showNonePwd()
{
    QMap<QString, QListWidgetItem*>::iterator iter;
    for(iter=m_wirelessNetItemMap.begin();iter!= m_wirelessNetItemMap.end();iter++)
    {
        if(iter.value() != nullptr) {
            QListWidgetItem *p_listWidgetItem = iter.value();
            WlanListItem *p_wlanItem = (WlanListItem*)m_inactivatedNetListWidget->itemWidget(p_listWidgetItem);
            p_wlanItem->forgetPwd();
            p_wlanItem->setExpanded(false);
        }
    }
    return;
}

void WlanPage::showRate()
{
    //定时获取网速
    setNetSpeed = new QTimer(this);
    setNetSpeed->setTimerType(Qt::PreciseTimer);
    connect(setNetSpeed, &QTimer::timeout, [&](){
        onSetNetSpeed(m_activatedNetListWidget, m_activateConnectionItemMap.contains(EMPTY_SSID), m_currentDevice);
    });
}

void WlanPage::showHiddenWlanPage(QString devName)
{
    if (m_joinHiddenWiFiPagePtrMap.contains(devName)) {
        if (m_joinHiddenWiFiPagePtrMap[devName] != nullptr) {
            qDebug() << "[WlanPage] showAddOtherWlanWidget" << devName << "already create,just raise";
            KWindowSystem::activateWindow(m_joinHiddenWiFiPagePtrMap[devName]->winId());
            KWindowSystem::raiseWindow(m_joinHiddenWiFiPagePtrMap[devName]->winId());
            return;
        }
    }

    JoinHiddenWiFiPage *hiddenWiFi =new JoinHiddenWiFiPage(devName);
    connect(hiddenWiFi, &JoinHiddenWiFiPage::hiddenWiFiPageClose, [&](QString interfaceName){
        if (m_joinHiddenWiFiPagePtrMap.contains(interfaceName)) {
            m_joinHiddenWiFiPagePtrMap[interfaceName] = nullptr;
        }
    });
    connect(hiddenWiFi, &JoinHiddenWiFiPage::showWlanList, this, &WlanPage::showMainWindow);

    m_joinHiddenWiFiPagePtrMap.insert(devName, hiddenWiFi);
    hiddenWiFi->show();
    KWindowSystem::raiseWindow(hiddenWiFi->winId());
    hiddenWiFi->centerToScreen();
}

void WlanPage::showDetailPage(QString devName, QString ssid)
{
    KyWirelessNetItem wirelessNetItem;
    if (!m_wirelessNetResource->getWifiNetwork(devName, ssid, wirelessNetItem)) {
        qDebug()<<"[WlanPage] " << ssid << " is missing when showDetailPage";
        return;
    }

    bool isActive = m_connectResource->isActivatedConnection(wirelessNetItem.m_connectUuid);

    if (m_wlanPagePtrMap.contains(devName) && m_wlanPagePtrMap[devName].contains(ssid)) {
        if (m_wlanPagePtrMap[devName][ssid] != nullptr) {
            qDebug() << "[WlanPage] ShowWlanDetailPage" << ssid << "already create,just raise";
            KWindowSystem::activateWindow(m_wlanPagePtrMap[devName][ssid]->winId());
            KWindowSystem::raiseWindow(m_wlanPagePtrMap[devName][ssid]->winId());
            return;
        }
    }

    NetDetail *netDetail = new NetDetail(devName, ssid, wirelessNetItem.m_connectUuid, isActive, true, !wirelessNetItem.m_isConfigured);
    m_wlanPagePtrMap[devName].insert(ssid, netDetail);
    connect(netDetail, &NetDetail::detailPageClose, [&](QString deviceName, QString wlanSsid){
        if (m_wlanPagePtrMap.contains(deviceName) && m_wlanPagePtrMap[deviceName].contains(wlanSsid)) {
            m_wlanPagePtrMap[deviceName][wlanSsid] = nullptr;
        }
    });
    netDetail->show();
    KWindowSystem::raiseWindow(netDetail->winId());
    netDetail->centerToScreen();
    kdk::WindowManager::setSkipSwitcher(netDetail->windowHandle(), true);
    kdk::WindowManager::setSkipTaskBar(netDetail->windowHandle(), true);
    kdk::WindowManager::setIconName(netDetail->windowHandle(), "lingmo-network");

    return;
}

void WlanPage::showAddOtherPage(QString devName)
{
    showHiddenWlanPage(devName);
}

bool WlanPage::checkWlanStatus(NetworkManager::ActiveConnection::State state)
{
    if (m_activatedConnectResource->checkWirelessStatus(state)) {
        return true;
    } else {
        return false;
    }
}

void WlanPage::setWirelessSwitchEnable(bool enable)
{
    qDebug() << "dbus setWirelessSwitchEnable = " << enable << __LINE__;
    setWirelessEnable(enable);
}

void WlanPage::getWirelessDeviceCap(QMap<QString, int> &map)
{
    for (int i = 0; i < m_devList.size(); ++i) {
        QString devName = m_devList.at(i);
        map.insert(devName, m_netDeviceResource->getWirelessDeviceCapability(devName));
    }
}

void WlanPage::getConnectivity(NetworkManager::Connectivity &connectivity)
{
    m_connectResource->getConnectivity(connectivity);
}

/**
 * @brief WlanPage::addWlanMoreItem
 * 添加‘更多网络’的条目
 */
void WlanPage::addWlanMoreItem()
{
    if (m_hiddenItem) {
        m_inactivatedNetListWidget->takeItem(m_inactivatedNetListWidget->row(m_hiddenItem));
        delete m_hiddenItem;
        m_hiddenItem = nullptr;
    }

    if (m_hiddenWlanWidget) {
//        disconnect(m_hiddenWlanWidget, &WlanMoreItem::hiddenWlanClicked, this, &WlanPage::onHiddenWlanClicked);
        m_hiddenWlanWidget->setParent(nullptr);
        delete m_hiddenWlanWidget;
        m_hiddenWlanWidget = nullptr;
    }

    m_hiddenWlanWidget = new WlanMoreItem(this);
    connect(m_hiddenWlanWidget, &WlanMoreItem::hiddenWlanClicked, this, &WlanPage::onHiddenWlanClicked);
    m_hiddenWlanWidget->setFixedHeight(NORMAL_HEIGHT);

    m_hiddenItem = new QListWidgetItem(m_inactivatedNetListWidget);
    m_hiddenItem->setFlags(m_hiddenItem->flags() & (~Qt::ItemIsSelectable));
    m_hiddenItem->setSizeHint(QSize(m_inactivatedNetListWidget->width(), m_hiddenWlanWidget->height()));
    m_inactivatedNetListWidget->addItem(m_hiddenItem);
    m_inactivatedNetListWidget->setItemWidget(m_hiddenItem, m_hiddenWlanWidget);
    m_hiddenWlanWidget->m_freq->hide();
    return;
}

int WlanPage::getActivateWifiSignal(QString devName)
{
    return m_activatedConnectResource->getActivateWifiSignal(devName);
}

void WlanPage::getWirelssDeviceConnectState(QMap<QString, QString> &map)
{
    map.clear();
    if (m_devList.isEmpty()) {
        return;
    }

    for (const auto devname : m_devList) {
        KyWirelessNetItem wirelessNetItem;
        if (!m_netDeviceResource->getDeviceManaged(devname)) {
            continue;
        }
        if (m_wirelessNetResource->getActiveWirelessNetItem(devname, wirelessNetItem)) {
            map.insert(devname, QString(tr("Connected: ")) + wirelessNetItem.m_NetSsid);
        } else {
            map.insert(devname, tr("Not Connected"));
        }
    }
}

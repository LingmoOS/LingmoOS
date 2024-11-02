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
#include "lanpage.h"
#include "kwindowsystem.h"
#include "kwindowsystem_export.h"
#include <QDebug>
#include <QScrollBar>

#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_SPACING 0
#define TITLE_FRAME_HEIGHT 52
#define TITLE_LAYOUT_MARGINS 24,0,24,0
#define LAN_LIST_SPACING 0
#define TEXT_MARGINS 16,0,0,0
#define SETTINGS_LAYOUT_MARGINS 24,16,24,16
#define TRANSPARENT_COLOR QColor(0,0,0,0)
#define ITEM_HEIGHT 48

#define LOG_FLAG "[LanPage]"
#include "windowmanager/windowmanager.h"
using namespace kdk;
const QString EMPTY_CONNECT_UUID = "emptyconnect";

const QString WIRED_SWITCH = "wiredswitch";

LanPage::LanPage(QWidget *parent) : TabPage(parent)
{
    m_activeResourse = new KyActiveConnectResourse(this);
    m_connectResourse = new KyConnectResourse(this);
    m_deviceResource = new KyNetworkDeviceResourse(this);
    m_wiredConnectOperation = new KyWiredConnectOperation(this);

    initUI();
    initLanDevice();
//    initNetSwitch();
    initLanDeviceState();

    initDeviceCombox();
    initLanArea();

    connect(m_activeResourse, &KyActiveConnectResourse::stateChangeReason, this, &LanPage::onConnectionStateChange);
    connect(m_activeResourse, &KyActiveConnectResourse::activeConnectRemove, this, [=] (QString activeConnectUuid) {
        sendLanStateChangeSignal(activeConnectUuid,Deactivated);
    } );

    connect(m_connectResourse, &KyConnectResourse::connectionAdd, this, &LanPage::onAddConnection);
    connect(m_connectResourse, &KyConnectResourse::connectionRemove, this, &LanPage::onRemoveConnection);
    connect(m_connectResourse, &KyConnectResourse::connectionUpdate, this, &LanPage::onUpdateConnection);

    connect(m_connectResourse, &KyConnectResourse::needShowDesktop, this, [=](QString ip) {
        this->showDesktopNotify(ip + tr("conflict, unable to connect to the network normally!"), QString());
    });

    connect(m_deviceResource, &KyNetworkDeviceResourse::deviceAdd, this, &LanPage::onDeviceAdd);
    connect(m_deviceResource, &KyNetworkDeviceResourse::deviceRemove, this, &LanPage::onDeviceRemove);
    connect(m_deviceResource, &KyNetworkDeviceResourse::deviceNameUpdate, this, &LanPage::onDeviceNameUpdate);

    connect(m_deviceResource, &KyNetworkDeviceResourse::carrierChanage, this, &LanPage::onDeviceCarriered);
    connect(m_deviceResource, &KyNetworkDeviceResourse::deviceActiveChanage, this, &LanPage::onDeviceActiveChanage);
    connect(m_deviceResource, &KyNetworkDeviceResourse::deviceManagedChange, this, &LanPage::onDeviceManagedChange);

    connect(m_wiredConnectOperation, &KyWiredConnectOperation::activateConnectionError, this, &LanPage::activateFailed);
    connect(m_wiredConnectOperation, &KyWiredConnectOperation::deactivateConnectionError, this, &LanPage::deactivateFailed);
    connect(m_wiredConnectOperation, &KyWiredConnectOperation::wiredEnabledChanged, this, &LanPage::onWiredEnabledChanged);
    m_netSwitch->hide();
//    connect(m_netSwitch, &KSwitchButton::clicked, this, [=](bool checked) {
//        m_netSwitch->setChecked(!checked);
//        m_wiredConnectOperation->setWiredEnabled(checked);
//    });
    m_lanPagePtrMap.clear();
}

LanPage::~LanPage()
{

}

void LanPage::initLanDevice()
{
    m_devList.clear();
    m_deviceResource->getNetworkDeviceList(NetworkManager::Device::Type::Ethernet, m_devList);

    m_currentDeviceName = getDefaultDeviceName(WIRED);
    bool getDefault = !m_currentDeviceName.isEmpty();
    if (getDefault) {
        if (m_deviceResource->wiredDeviceIsCarriered(m_currentDeviceName)) {
            return;
        }
    }

    QList<KyConnectItem *> activedList;
    for (int index = 0; index < m_devList.size(); ++index) {
        m_activeResourse->getActiveConnectionList(m_devList.at(index),
                                   NetworkManager::ConnectionSettings::Wired, activedList);
        if (!activedList.isEmpty()) {
            m_currentDeviceName = m_devList.at(index);
            if (!getDefault) {
                setDefaultDevice(WIRED, m_currentDeviceName);
            }
            return;
        }
    }

    for (int index = 0; index < m_devList.size(); ++index) {
        if (m_deviceResource->wiredDeviceIsCarriered(m_devList.at(index))) {
            m_currentDeviceName = m_devList.at(index);
            if (!getDefault) {
                setDefaultDevice(WIRED, m_currentDeviceName);
            }
            return;
        }
    }
    return;
}

void LanPage::initLanDeviceState()
{
    m_disableDeviceList.clear();
    m_enableDeviceList.clear();
    for (int index = 0; index < m_devList.count(); ++index) {
        QString deviceName = m_devList.at(index);
        if (m_deviceResource->getDeviceManaged(deviceName)) {
            m_enableDeviceList<<deviceName;
        } else {
            m_disableDeviceList<<deviceName;
        }
    }
}

void LanPage::initNetSwitch()
{
//    bool wiredGsetting = true;
//    bool wiredEnable = m_wiredConnectOperation->getWiredEnabled();

//    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
//        m_switchGsettings = new QGSettings(GSETTINGS_SCHEMA);
//        if (m_switchGsettings->keys().contains(WIRED_SWITCH)) {
//            wiredGsetting = m_switchGsettings->get(WIRED_SWITCH).toBool();
//            connect(m_switchGsettings, &QGSettings::changed, this, &LanPage::onSwithGsettingsChanged);
//            if (wiredEnable != wiredGsetting) {
//                m_switchGsettings->set(WIRED_SWITCH, wiredEnable);
//            }
//        }
//    } else {
//        qDebug()<<"[LanPage] org.lingmo.lingmo-nm.switch is not installed!";
//    }

//    //从3.0升级上来 先读取老的配置文件来保证和升级前状态一致
//    bool oldVersionState = true;
//    if (getOldVersionWiredSwitchState(oldVersionState)) {
//        if (wiredEnable != oldVersionState) {
//            wiredEnable = oldVersionState;
//            m_wiredConnectOperation->setWiredEnabled(oldVersionState);
//        }
//    }

//    if (m_devList.count() == 0) {
//        qDebug() << "[wiredSwitch]:init not enable when no device";
//        m_netSwitch->setChecked(false);
//        m_netSwitch->setCheckable(false);
//    }

//    qDebug() << "[wiredSwitch]:init state:" << wiredEnable;

//    m_netSwitch->setChecked(wiredEnable);
}

void LanPage::onSwithGsettingsChanged(const QString &key)
{
//    if (key == WIRED_SWITCH) {

//        bool wiredSwitch = m_switchGsettings->get(WIRED_SWITCH).toBool();
//        qDebug()<<"[LanPage] SwitchButton statue changed to:" << wiredSwitch << m_netSwitch->isChecked();

//        if (wiredSwitch != m_wiredConnectOperation->getWiredEnabled()) {
//            m_wiredConnectOperation->setWiredEnabled(wiredSwitch);
//            return;
//        }

//        m_netSwitch->setChecked(wiredSwitch);

//        initLanDeviceState();
//        initDeviceCombox();
//        initLanArea();
//    }
}

void LanPage::getEnabledDevice(QStringList &enableDeviceList)
{
    if (m_devList.isEmpty()) {
        qDebug()<<"[LanPage] there is not wired device.";
        return;
    }

    for (int index = 0; index < m_devList.size(); ++index) {
        if (m_deviceResource->getDeviceManaged(m_devList.at(index))) {
            enableDeviceList << m_devList.at(index);
        }
    }
}

void LanPage::getDisabledDevices(QStringList &disableDeviceList)
{
    if (m_devList.isEmpty()) {
        qDebug()<<"[LanPage] there is not wired device.";
        return;
    }

    for (int index = 0; index < m_devList.size(); ++index) {
        if (!m_deviceResource->getDeviceManaged(m_devList.at(index))) {
            disableDeviceList << m_devList.at(index);
        }
    }
}

void LanPage::initDeviceCombox()
{
    //TODO 获取设备列表，单设备时隐藏下拉框，多设备时添加到下拉框;m_devList记录插入的所有设备，deviceMap记录设备状态
    disconnect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                this, &LanPage::onDeviceComboxIndexChanged);

    m_deviceComboBox->clear();

    if (!m_devList.isEmpty()) {
        int enableDeviceCount = m_enableDeviceList.count();
        if (enableDeviceCount > 1) {
            for (int index = 0; index < enableDeviceCount; ++index) {
                m_deviceComboBox->addItem(m_enableDeviceList.at(index));
            }

            m_deviceFrame->show();
            m_tipsLabel->hide();
            m_deviceComboBox->show();

            if (m_currentDeviceName != m_deviceComboBox->currentText()) {
                if (m_enableDeviceList.contains(m_currentDeviceName)) {
                    m_deviceComboBox->setCurrentText(m_currentDeviceName);
                } else {
                    m_currentDeviceName = m_deviceComboBox->currentText();
                }
            }

        } else if (enableDeviceCount == 1) {
            m_deviceFrame->hide();

            if (m_currentDeviceName != m_enableDeviceList.at(0)) {
                m_currentDeviceName = m_enableDeviceList.at(0);
            }
        } else {
            m_deviceFrame->show();
            m_deviceComboBox->hide();
            m_tipsLabel->show();
            m_currentDeviceName = "";
        }
    } else {
        m_deviceFrame->hide();
        m_currentDeviceName = "";
    }

    setDefaultDevice(WIRED, m_currentDeviceName);
    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LanPage::onDeviceComboxIndexChanged);
}

void LanPage::addEmptyConnectItem(QMap<QString, QListWidgetItem *> &connectMap,
                                       QListWidget *lanListWidget)
{
    QListWidgetItem *p_listWidgetItem = addNewItem(nullptr, lanListWidget);
    connectMap.insert(EMPTY_CONNECT_UUID, p_listWidgetItem);
}


void LanPage::deleteConnectionMapItem(QMap<QString, QListWidgetItem *> &connectMap,
                             QListWidget *lanListWidget, QString uuid)
{
    QListWidgetItem *p_listWidgetItem = connectMap.value(uuid);
    if (p_listWidgetItem) {
        connectMap.remove(uuid);
        LanListItem *p_lanItem = (LanListItem *)lanListWidget->itemWidget(p_listWidgetItem);
        lanListWidget->removeItemWidget(p_listWidgetItem);

        delete p_lanItem;
        p_lanItem = nullptr;

        delete p_listWidgetItem;
        p_listWidgetItem = nullptr;
    }

    return;
}

void LanPage::clearConnectionMap(QMap<QString, QListWidgetItem *> &connectMap,
                                 QListWidget *lanListWidget)
{
    QMap<QString, QListWidgetItem *>::iterator iter;

    iter = connectMap.begin();
    while (iter != connectMap.end()) {
        qDebug()<<"[LanPage] clear connection map item"<< iter.key();

        QListWidgetItem *p_widgetItem = iter.value();
        LanListItem *p_lanItem = (LanListItem *)lanListWidget->itemWidget(p_widgetItem);
        lanListWidget->removeItemWidget(p_widgetItem);

        delete p_lanItem;
        p_lanItem = nullptr;

        delete p_widgetItem;
        p_widgetItem = nullptr;

        iter = connectMap.erase(iter);
    }

    return;
}

void LanPage::constructActiveConnectionArea()
{
    QList<KyConnectItem *> activedList;

    activedList.clear();
    clearConnectionMap(m_activeConnectionMap, m_activatedLanListWidget);

    m_activeResourse->getActiveConnectionList(m_currentDeviceName,
                               NetworkManager::ConnectionSettings::Wired, activedList);      //激活列表的显示
    qDebug() << "[LanPage] construct active connection area get Active list size:" << activedList.size();
    if (!activedList.isEmpty()) {
        for (int index = 0; index < activedList.size(); index++) {
            KyConnectItem *p_activeConnectionItem = activedList.at(index);
            qDebug() << "[LanPage] construct active connection area. add active item"
                     << p_activeConnectionItem->m_connectName;
            QListWidgetItem *p_listWidgetItem = addNewItem(p_activeConnectionItem, m_activatedLanListWidget);
            m_activeConnectionMap.insert(p_activeConnectionItem->m_connectUuid, p_listWidgetItem);

            delete p_activeConnectionItem;
            p_activeConnectionItem = nullptr;
        }
        setNetSpeed->start(REFRESH_NETWORKSPEED_TIMER);
        m_activatedLanListWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    } else {
        qDebug()<<"[LanPage] there is not active";
        addEmptyConnectItem(m_activeConnectionMap, m_activatedLanListWidget);
        m_activatedLanListWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    }

    return;
}

void LanPage::constructConnectionArea()
{
    QList<KyConnectItem *> deactivedList;

    deactivedList.clear();
    clearConnectionMap(m_inactiveConnectionMap, m_inactivatedLanListWidget);

    m_connectResourse->getConnectionList(m_currentDeviceName, NetworkManager::ConnectionSettings::Wired, deactivedList);      //未激活列表的显示
    qDebug() << "[LanPage]construct connection area get connection list size:" << deactivedList.size();
    if (!deactivedList.isEmpty()) {
        for (int index = 0; index < deactivedList.size(); index++) {
            KyConnectItem *p_deactiveConnectionItem = deactivedList.at(index);
            qDebug()<<"[LanPage] construct connection area add deactive item"<<p_deactiveConnectionItem->m_connectName;
            QListWidgetItem *p_listWidgetItem = addNewItem(p_deactiveConnectionItem, m_inactivatedLanListWidget);
            if (m_inactiveConnectionMap.contains(p_deactiveConnectionItem->m_connectUuid)) {
                qDebug()<<LOG_FLAG << "has contain uuid" << p_deactiveConnectionItem->m_connectUuid;
            }
            m_inactiveConnectionMap.insert(p_deactiveConnectionItem->m_connectUuid, p_listWidgetItem);

            delete p_deactiveConnectionItem;
            p_deactiveConnectionItem = nullptr;

        }
    }
    if (m_inactivatedLanListWidget->count() <= MAX_ITEMS) {
        m_inactivatedLanListWidget->setFixedWidth(MIN_WIDTH);
    } else {
        m_inactivatedLanListWidget->setFixedWidth(MAX_WIDTH);
    }
    return;
}

void LanPage::initLanArea()
{
    if (/*!m_netSwitch->isChecked() || */m_currentDeviceName.isEmpty()) {
        m_activatedNetDivider->hide();
        m_activatedNetFrame->hide();

        m_inactivatedNetFrame->hide();
    } else {
        m_activatedNetDivider->show();
        m_activatedNetFrame->show();
        constructActiveConnectionArea();

        m_inactivatedNetFrame->show();
        constructConnectionArea();
    }

    if (!m_activeConnectionMap.isEmpty() && !m_activeConnectionMap.contains(EMPTY_CONNECT_UUID) && !setNetSpeed->isActive()) {
        setNetSpeed->start();
    } else {
        setNetSpeed->stop();
    }
}

bool LanPage::removeConnectionItem(QMap<QString, QListWidgetItem *> &connectMap,
                          QListWidget *lanListWidget, QString path)
{
    QMap<QString, QListWidgetItem *>::iterator iter;
    for (iter = connectMap.begin(); iter != connectMap.end(); ++iter) {
        QListWidgetItem *p_listWidgetItem = iter.value();
        LanListItem *p_lanItem = (LanListItem*)lanListWidget->itemWidget(p_listWidgetItem);
        if (p_lanItem->getConnectionPath() == path) {
            qDebug()<<"[LanPage] Remove a connection from list";

            lanListWidget->removeItemWidget(p_listWidgetItem);

            delete p_lanItem;
            p_lanItem = nullptr;

            delete p_listWidgetItem;
            p_listWidgetItem = nullptr;

            iter = connectMap.erase(iter);
            if (m_inactivatedLanListWidget->count() <= MAX_ITEMS) {
                m_inactivatedLanListWidget->setFixedWidth(MIN_WIDTH);
            }
            return true;
        }
    }

    return false;
}

void LanPage::onRemoveConnection(QString path)            //删除时后端会自动断开激活，将其从未激活列表中删除
{
    //for dbus
    qDebug() << "[LanPage] Q_EMIT lanRemove because onRemoveConnection " << path;
    Q_EMIT lanRemove(path);

    if (m_lanPagePtrMap.contains(path)) {
        if (m_lanPagePtrMap[path] != nullptr) {
            delete m_lanPagePtrMap[path];
            m_lanPagePtrMap[path] = nullptr;
        }
    }

    if (removeConnectionItem(m_inactiveConnectionMap, m_inactivatedLanListWidget, path)) {
        return;
    } else {
        removeConnectionItem(m_activeConnectionMap, m_activatedLanListWidget, path);
        if (m_activeConnectionMap.count() <= 0) {
            addEmptyConnectItem(m_activeConnectionMap, m_activatedLanListWidget);
            m_activatedLanListWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
        }
        setNetSpeed->stop();
        return;
    }
}

void LanPage::onAddConnection(QString uuid)               //新增一个有线连接，将其加入到激活列表
{
    if (!m_connectResourse->isWiredConnection(uuid)) {
        return;
    }

    KyConnectItem *p_newItem = nullptr;
    p_newItem = m_connectResourse->getConnectionItemByUuid(uuid);
    if (nullptr == p_newItem) {
        return;
    }

    sendLanAddSignal(p_newItem);

    if (p_newItem->m_ifaceName == m_currentDeviceName || p_newItem->m_ifaceName == "") {
        qDebug()<<"[LanPage] Add a new connection, name:"<<p_newItem->m_connectName;
        QListWidgetItem *p_listWidgetItem = insertNewItem(p_newItem, m_inactivatedLanListWidget);
        if (m_inactiveConnectionMap.contains(p_newItem->m_connectUuid)) {
            qDebug()<<LOG_FLAG << "the connection is exsit" << p_newItem->m_connectUuid;
        }
        m_inactiveConnectionMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);
    }

    delete p_newItem;
    p_newItem = nullptr;
    if (m_inactivatedLanListWidget->count() > MAX_ITEMS) {
        m_inactivatedLanListWidget->setFixedWidth(MAX_WIDTH);
    }
}

void LanPage::addDeviceForCombox(QString deviceName)
{
    disconnect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this, &LanPage::onDeviceComboxIndexChanged);

//    if (m_netSwitch->isChecked()) {
        if (1 == m_enableDeviceList.count()) {
            //1、从无到有添加第一块有线网卡
            //2、有多快网卡，但是没有使能
            m_deviceFrame->hide();
            m_currentDeviceName = deviceName;
            setDefaultDevice(WIRED, m_currentDeviceName);
        } else if (2 == m_enableDeviceList.count()) {
            //3、现在有且只有一块网卡，并已使能
            //4、有多快网卡，且使能了其中一块
            m_deviceComboBox->addItem(m_currentDeviceName);
            m_deviceComboBox->addItem(deviceName);

            m_deviceFrame->show();
            m_tipsLabel->hide();
            m_deviceComboBox->show();
        } else if (m_enableDeviceList.count() > 2) {
            //5、有多快网卡且使能了多块网卡
            m_deviceComboBox->addItem(deviceName);
        }
//    }

    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LanPage::onDeviceComboxIndexChanged);
}

void LanPage::onDeviceAdd(QString deviceName, NetworkManager::Device::Type deviceType)
{
    Q_UNUSED(deviceType)
    if (!m_deviceResource->checkDeviceType(deviceName, NetworkManager::Device::Type::Ethernet)) {
        return;
    }

    if (m_devList.contains(deviceName)) {
        return;
    }

//    if (m_devList.count() == 0) {// 有线网卡从无到有，打开开关
//        bool wiredSwitch = m_switchGsettings->get(WIRED_SWITCH).toBool();
//        m_netSwitch->setCheckable(true);
//        m_netSwitch->setChecked(wiredSwitch);
//    }

    qDebug() << "[LanPage] Begin add device:" << deviceName;

    m_devList << deviceName;
    if (m_deviceResource->getDeviceManaged(deviceName)) {
        m_enableDeviceList << deviceName;
        addDeviceForCombox(deviceName);
        if (m_currentDeviceName == deviceName) {
            initLanArea();
        }
    } else {
        m_disableDeviceList << deviceName;
    }

    Q_EMIT deviceStatusChanged();
}

void LanPage::deleteDeviceFromCombox(QString deviceName)
{
    disconnect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, &LanPage::onDeviceComboxIndexChanged);


    if (m_devList.isEmpty()) {
        m_tipsLabel->hide();
        m_deviceFrame->hide();
        m_deviceComboBox->hide();
    }
//    if (m_netSwitch->isChecked()) {
        if (0 == m_enableDeviceList.count()) {
            //1、没有使能任何网卡
            goto l_out;
        } else if (1 == m_enableDeviceList.count()) {
             //2、使能了一个网卡，且当前网卡是要删除的网卡
            if (m_currentDeviceName == deviceName) {
                m_deviceFrame->show();
                m_deviceComboBox->hide();
                m_tipsLabel->show();
                m_currentDeviceName = "";
                setDefaultDevice(WIRED, m_currentDeviceName);
            }
        } else if (2 == m_enableDeviceList.count()) {
            //3、使能了两个网卡，且包括要删除的网卡，有可能是要删除的网卡
            if (m_deviceComboBox->findText(deviceName) != -1) {
                for (int index = m_deviceComboBox->count() - 1; index >= 0; index--) {
                    if (m_currentDeviceName == deviceName
                            && m_deviceComboBox->itemText(index) != deviceName) {
                        m_currentDeviceName = m_deviceComboBox->itemText(index);
                        setDefaultDevice(WIRED, m_currentDeviceName);
                    }
                    m_deviceComboBox->removeItem(index);
                }

                m_tipsLabel->hide();
                m_deviceFrame->hide();
                m_deviceComboBox->hide();
            }
        } else {
            //4、使能了多个网卡，且包括要删除的网卡，有可能是正要删除的网卡
            int index = m_deviceComboBox->findText(deviceName);
            if (index != -1) {
                m_deviceComboBox->removeItem(index);
                if (m_currentDeviceName == deviceName) {
                    m_currentDeviceName = m_deviceComboBox->currentText();
                    setDefaultDevice(WIRED, m_currentDeviceName);
                }
            }
        }
//    }

l_out:
    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LanPage::onDeviceComboxIndexChanged);

    return;
}

void LanPage::onDeviceRemove(QString deviceName)
{
    if (!m_devList.contains(deviceName)) {
        return;
    }

    qDebug() << "[LanPage] deviceRemove:" << deviceName;

    m_devList.removeOne(deviceName);
//    if (m_devList.count() == 0) {
//        m_netSwitch->setChecked(false);
//        m_netSwitch->setCheckable(false);
//        qDebug() << "[wiredSwitch]set not enable after device remove";
//    }

    QString nowDevice = m_currentDeviceName;
    deleteDeviceFromCombox(deviceName);
    if (nowDevice == deviceName) {
        setDefaultDevice(WIRED, m_currentDeviceName);
        initLanArea();
    }

    if (m_enableDeviceList.contains(deviceName)) {
        m_enableDeviceList.removeOne(deviceName);
    }
    if (m_disableDeviceList.contains(deviceName)) {
        m_disableDeviceList.removeOne(deviceName);
    }

    Q_EMIT deviceStatusChanged();
}

void LanPage::updateDeviceCombox(QString oldDeviceName, QString newDeviceName)
{
    if (m_currentDeviceName == oldDeviceName) {
        m_currentDeviceName = newDeviceName;
        setDefaultDevice(WIRED, m_currentDeviceName);
    }

    int index = m_deviceComboBox->findText(oldDeviceName);
    if (index != -1) {
        m_deviceComboBox->setItemText(index, newDeviceName);
    }

    return;
}

void LanPage::onDeviceNameUpdate(QString oldName, QString newName)
{
    if (m_devList.contains(oldName)) {
        m_devList.removeOne(oldName);
        m_devList.append(newName);

        if (m_enableDeviceList.contains(oldName)) {
            m_enableDeviceList.removeOne(oldName);
            m_enableDeviceList.append(newName);
        } else  if (m_disableDeviceList.contains(oldName)) {
            m_disableDeviceList.removeOne(oldName);
            m_disableDeviceList.append(newName);
        }
        qDebug() << "[LanPage] Q_EMIT deviceNameUpdate "  << oldName << newName;

        updateDeviceCombox(oldName, newName);
        if (m_currentDeviceName == newName) {
            initLanArea();
        }

        Q_EMIT deviceNameChanged(oldName, newName, WIRED);
    }
}

void LanPage::onDeviceCarriered(QString deviceName, bool pluged)
{
    if (!pluged) {
        return;
    }
    if (m_enableDeviceList.contains(deviceName)) {
        updateCurrentDevice(deviceName);
    }
}

void LanPage::onDeviceActiveChanage(QString deviceName, bool deviceActive)
{
//    if (!m_devList.contains(deviceName)) {
//        return;
//    }

//    if (deviceActive) {
//        if (!m_netSwitch->isChecked() || !m_enableDeviceList.contains(deviceName)) {
//            qDebug()<< LOG_FLAG << "close disabled device";
//            m_wiredConnectOperation->closeWiredNetworkWithDevice(deviceName);
//        }
//    }

//    return;
}

void LanPage::onDeviceManagedChange(QString deviceName, bool managed)
{
    initLanDeviceState();
    initDeviceCombox();
    initLanArea();
    Q_EMIT deviceStatusChanged();
}


void LanPage::onDeviceComboxIndexChanged(int currentIndex)
{
    //TODO 设备变更时更新设备和列表
    m_currentDeviceName = m_deviceComboBox->currentText();
    qDebug() << "[LanPage]Current Device Changed to:" << m_currentDeviceName;
    initLanArea();
}

void LanPage::onShowControlCenter()
{
    QProcess process;
    process.startDetached("lingmo-control-center -m netconnect");
}

void LanPage::initUI()
{
    m_titleLabel->setText(tr("LAN"));

    m_activatedNetLabel->setText(tr("Activated LAN"));
    m_activatedLanListWidget = new QListWidget(m_activatedNetFrame);
    m_activatedLanListWidget->setFrameShape(QFrame::Shape::NoFrame);
    m_activatedLanListWidget->setSpacing(LAN_LIST_SPACING);
    m_activatedLanListWidget->setFixedHeight(ITEM_HEIGHT);              //active区域固定高度,只显示一个条目
    m_activatedLanListWidget->setFixedWidth(MIN_WIDTH);
    m_activatedLanListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_activatedLanListWidget->setProperty("needTranslucent", true);
    m_activatedNetLayout->addWidget(m_activatedLanListWidget);

    m_inactivatedNetLabel->setText(tr("Inactivated LAN"));
    m_inactivatedLanListWidget = new QListWidget(m_inactivatedNetListArea);
    m_inactivatedLanListWidget->setFrameShape(QFrame::Shape::NoFrame);
    m_inactivatedLanListWidget->setSpacing(LAN_LIST_SPACING);
    m_inactivatedLanListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inactivatedLanListWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    m_inactivatedLanListWidget->verticalScrollBar()->setProperty("drawScrollBarGroove",false); //去除滚动条的外侧黑框
    m_inactivatedLanListWidget->verticalScrollBar()->setSingleStep(SCROLL_STEP);
    m_inactivatedLanListWidget->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_inactivatedLanListWidget->verticalScrollBar()->setProperty("needTranslucent", true);
    m_inactivatedLanListWidget->setProperty("needTranslucent", true);
    m_inactivatedAreaLayout->addWidget(m_inactivatedLanListWidget);

    connect(m_inactivatedLanListWidget, &QListWidget::currentItemChanged, this, [=]() {
        if (m_inactivatedLanListWidget->currentItem() != nullptr) {
            m_inactivatedLanListWidget->currentItem()->setSelected(false);
        }
    });

    QPalette pal = m_activatedLanListWidget->palette();
    pal.setBrush(QPalette::Base, QColor(0,0,0,0));        //背景透明
    m_activatedLanListWidget->setPalette(pal);
    m_inactivatedLanListWidget->setPalette(pal);

    m_settingsLabel->installEventFilter(this);
//    m_netSwitch->installEventFilter(this);
    m_activatedLanListWidget->installEventFilter(this);
    m_inactivatedLanListWidget->installEventFilter(this);

    showRate();
}

QListWidgetItem *LanPage::insertNewItem(KyConnectItem *itemData, QListWidget *listWidget)
{
    int index = 0;

    for(index = 0; index < m_inactivatedLanListWidget->count(); index++) {
        QListWidgetItem *p_listWidgetItem = m_inactivatedLanListWidget->item(index);
        LanListItem *p_lanItem = (LanListItem *)m_inactivatedLanListWidget->itemWidget(p_listWidgetItem);
        QString name1 = p_lanItem->getConnectionName();
        QString name2 = itemData->m_connectName;
        if (QString::compare(name1, name2, Qt::CaseInsensitive) > 0) {
            break;
        }
    }

    QListWidgetItem *p_sortListWidgetItem = new QListWidgetItem();
//    p_sortListWidgetItem->setFlags(p_sortListWidgetItem->flags() & (~Qt::ItemIsSelectable));   //设置不可被选中
    p_sortListWidgetItem->setSizeHint(QSize(listWidget->width(),ITEM_HEIGHT));

    listWidget->insertItem(index, p_sortListWidgetItem);

    LanListItem *p_sortLanItem = nullptr;
    p_sortLanItem = new LanListItem(itemData, m_currentDeviceName);
    listWidget->setItemWidget(p_sortListWidgetItem, p_sortLanItem);
    connect(p_sortLanItem, &LanListItem::detailShow, this, &LanPage::showDetailPage);
    return p_sortListWidgetItem;
}

QListWidgetItem *LanPage::addNewItem(KyConnectItem *itemData, QListWidget *listWidget)
{
    QListWidgetItem *p_listWidgetItem = new QListWidgetItem();
//    p_listWidgetItem->setFlags(p_listWidgetItem->flags() & (~Qt::ItemIsSelectable));
    p_listWidgetItem->setSizeHint(QSize(listWidget->width(), ITEM_HEIGHT));
    listWidget->addItem(p_listWidgetItem);
    LanListItem *p_lanItem = nullptr;
    if (itemData != nullptr) {
        p_lanItem = new LanListItem(itemData, m_currentDeviceName);
        qDebug() << "[LanPage] addNewItem, connection: "
                 << itemData->m_connectName << "deviceName: " << m_currentDeviceName;
    } else {
        p_lanItem = new LanListItem();
        qDebug() << "[LanPage] Add nullItem!";
    }

    listWidget->setItemWidget(p_listWidgetItem, p_lanItem);
    connect(p_lanItem, &LanListItem::detailShow, this, &LanPage::showDetailPage);
    return p_listWidgetItem;
}

void LanPage::updateActivatedConnectionArea(KyConnectItem *p_newItem)
{
    m_activatedLanListWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    if (m_activeConnectionMap.contains(p_newItem->m_connectUuid)) {
        return;
    }

    deleteConnectionMapItem(m_inactiveConnectionMap, m_inactivatedLanListWidget, p_newItem->m_connectUuid);
    if (p_newItem->m_ifaceName == m_currentDeviceName) {
        qDebug()<<"[LanPage]update active connection item"<<p_newItem->m_connectName;
        deleteConnectionMapItem(m_activeConnectionMap, m_activatedLanListWidget, EMPTY_CONNECT_UUID);
        QListWidgetItem *p_listWidgetItem = addNewItem(p_newItem, m_activatedLanListWidget);
        m_activeConnectionMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);
//        this->showDesktopNotify(tr("LAN Connected Successfully"), "networkconnected");
    }
    if (m_inactivatedLanListWidget->count() <= MAX_ITEMS) {
        m_inactivatedLanListWidget->setFixedWidth(MIN_WIDTH);
    }

    return;
}

void LanPage::updateConnectionArea(KyConnectItem *p_newItem)
{
    if (m_inactiveConnectionMap.contains(p_newItem->m_connectUuid)) {
        return;
    }

    deleteConnectionMapItem(m_activeConnectionMap, m_activatedLanListWidget, p_newItem->m_connectUuid);
    if (m_activeConnectionMap.count() <= 0) {
        addEmptyConnectItem(m_activeConnectionMap, m_activatedLanListWidget);
        m_activatedLanListWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    }

    if (p_newItem->m_ifaceName == m_currentDeviceName || p_newItem->m_ifaceName == "") {
        qDebug()<<"[LanPage] update connection item"<<p_newItem->m_connectName;
        QListWidgetItem *p_listWidgetItem = insertNewItem(p_newItem, m_inactivatedLanListWidget);
        m_inactiveConnectionMap.insert(p_newItem->m_connectUuid, p_listWidgetItem);
//        this->showDesktopNotify(tr("LAN Disconnected Successfully"), "networkdisconnected");
    }
    if (m_inactivatedLanListWidget->count() <= MAX_ITEMS) {
        m_inactivatedLanListWidget->setFixedWidth(MIN_WIDTH);
    } else {
        m_inactivatedLanListWidget->setFixedWidth(MAX_WIDTH);
    }

    return;
}

void LanPage::updateConnectionState(QMap<QString, QListWidgetItem *> &connectMap,
                                    QListWidget *lanListWidget, QString uuid, ConnectState state)
{
    qDebug() << LOG_FLAG << "update connection state";

    QListWidgetItem *p_listWidgetItem = connectMap.value(uuid);
    if (p_listWidgetItem) {
        LanListItem *p_lanItem = (LanListItem *)lanListWidget->itemWidget(p_listWidgetItem);
        p_lanItem->updateConnectionState(state);
    }

    return;
}

QString LanPage::getConnectionDevice(QString uuid)
{
    QString deviceName = "";

    deviceName = m_activeResourse->getDeviceOfActivateConnect(uuid);
    if (deviceName.isEmpty()) {
        m_connectResourse->getInterfaceByUuid(deviceName, uuid);
    }

    return deviceName;
}

void LanPage::updateCurrentDevice(QString deviceName)
{
    if (m_currentDeviceName != deviceName) {
        int index = m_deviceComboBox->findText(deviceName);
        if (index < 0) {
            index = 0;
        }
        m_deviceComboBox->setCurrentIndex(index);
        return;
    }
    return;
}

void LanPage::onConnectionStateChange(QString uuid,
                              NetworkManager::ActiveConnection::State state,
                              NetworkManager::ActiveConnection::Reason reason)
{
    //lanpage函数内持续监听连接状态的变化并记录供其他函数调用获取状态
    if (!m_connectResourse->isWiredConnection(uuid)) {
        qDebug() << "[LanPage] connection state change signal but not wired";
        return;
    }

    sendLanStateChangeSignal(uuid, (ConnectState)state);

    if (m_activeConnectionMap.keys().contains(uuid) && state == NetworkManager::ActiveConnection::State::Activated) {
        return;
    }

    qDebug()<<"[LanPage] connection uuid"<< uuid
            << "state change slot:"<< state;

    KyConnectItem *p_newItem = nullptr;
    QString deviceName = "";

    if (state == NetworkManager::ActiveConnection::State::Activated) {
        p_newItem = m_activeResourse->getActiveConnectionByUuid(uuid);
        if (nullptr == p_newItem) {
            qWarning()<<"[LanPage] get active connection failed, connection uuid" << uuid;
            return;
        }
        updateActivatedConnectionArea(p_newItem);
        updateConnectionState(m_activeConnectionMap, m_activatedLanListWidget, uuid, (ConnectState)state);
        deviceName = p_newItem->m_ifaceName;
        setNetSpeed->start(REFRESH_NETWORKSPEED_TIMER);
    } else if (state == NetworkManager::ActiveConnection::State::Deactivated) {
        p_newItem = m_connectResourse->getConnectionItemByUuidWithoutActivateChecking(uuid);
        qDebug() << "[LanPage] deactivated reason" << reason;
        if (nullptr == p_newItem) {
            qWarning()<<"[LanPage] get active connection failed, connection uuid" << uuid;
            return;
        }

        deviceName = p_newItem->m_ifaceName;
        updateConnectionArea(p_newItem);
        updateConnectionState(m_inactiveConnectionMap, m_inactivatedLanListWidget, uuid, (ConnectState)state);
        setNetSpeed->stop();
    } else if (state == NetworkManager::ActiveConnection::State::Activating) {
        deviceName = getConnectionDevice(uuid);
        if (deviceName == m_currentDeviceName) {
            updateConnectionState(m_inactiveConnectionMap, m_inactivatedLanListWidget, uuid, (ConnectState)state);
        }
    } else if (state == NetworkManager::ActiveConnection::State::Deactivating) {
        deviceName = getConnectionDevice(uuid);
        if (deviceName == m_currentDeviceName) {
            updateConnectionState(m_activeConnectionMap, m_activatedLanListWidget, uuid, (ConnectState)state);
        }
    }

    Q_EMIT lanActiveConnectionStateChanged(deviceName, uuid, state);

    if (p_newItem) {
        delete p_newItem;
        p_newItem = nullptr;
    }

    return;
}


void LanPage::getWiredList(QString devName, QList<QStringList> &list)
{
    QStringList devlist;
    m_deviceResource->getNetworkDeviceList(NetworkManager::Device::Type::Ethernet, devlist);
    if (!devlist.contains(devName)) {
        return;
    }

    QList<KyConnectItem *> activedList;
    QList<KyConnectItem *> deactivedList;
    m_activeResourse->getActiveConnectionList(devName,NetworkManager::ConnectionSettings::Wired,activedList);
    if (!activedList.isEmpty()) {
        list.append(QStringList() << activedList.at(0)->m_connectName << activedList.at(0)->m_connectUuid << activedList.at(0)->m_connectPath);
    } else {
        list.append(QStringList()<<("--"));
    }

    m_connectResourse->getConnectionList(devName, NetworkManager::ConnectionSettings::Wired, deactivedList);      //未激活列表的显示
    if (!deactivedList.isEmpty()) {
        for (int i = 0; i < deactivedList.size(); i++) {
            list.append(QStringList()<<deactivedList.at(i)->m_connectName<<deactivedList.at(i)->m_connectUuid << deactivedList.at(i)->m_connectPath);
        }
    }
}

void LanPage::sendLanUpdateSignal(KyConnectItem *p_connectItem)
{
    QStringList info;
    info << p_connectItem->m_connectName << p_connectItem->m_connectUuid << p_connectItem->m_connectPath;
    Q_EMIT lanUpdate(p_connectItem->m_ifaceName, info);

    return;
}

void LanPage::sendLanAddSignal(KyConnectItem *p_connectItem)
{
    QStringList info;
    info << p_connectItem->m_connectName << p_connectItem->m_connectUuid << p_connectItem->m_connectPath;
    qDebug() << "[LanPage] Q_EMIT lanAdd because addConnection ";
    Q_EMIT lanAdd(p_connectItem->m_ifaceName, info);

    return;
}

void LanPage::sendLanStateChangeSignal(QString uuid, ConnectState state)
{
    if (state == Activating || state == Deactivating) {
        if (m_activeResourse->connectionIsVirtual(uuid)) {
            return;
        }
    }
    Q_EMIT this->lanConnectChanged(state);

    return;
}

void LanPage::updateConnectionProperty(KyConnectItem *p_connectItem)
{
    QString newUuid = p_connectItem->m_connectUuid;

    if (m_inactiveConnectionMap.contains(newUuid)) {
        QListWidgetItem *p_listWidgetItem = m_inactiveConnectionMap.value(newUuid);
        LanListItem *p_lanItem = (LanListItem*)m_inactivatedLanListWidget->itemWidget(p_listWidgetItem);
        if (p_connectItem->m_ifaceName != ""
                && m_currentDeviceName != p_connectItem->m_ifaceName) {
            m_inactivatedLanListWidget->removeItemWidget(p_listWidgetItem);

            delete p_listWidgetItem;
            p_listWidgetItem = nullptr;

            delete p_lanItem;
            p_lanItem = nullptr;

            m_inactiveConnectionMap.remove(newUuid);
        } else {
            if (p_connectItem->m_connectName != p_lanItem->getConnectionName()){
                //只要名字改变就要删除，重新插入，主要是为了排序
                deleteConnectionMapItem(m_inactiveConnectionMap, m_inactivatedLanListWidget, newUuid);
                QListWidgetItem *p_sortListWidgetItem = insertNewItem(p_connectItem, m_inactivatedLanListWidget);
                if (m_inactiveConnectionMap.contains(newUuid)) {
                    qDebug()<<LOG_FLAG << "has contained connection" << newUuid;
                }
                m_inactiveConnectionMap.insert(newUuid, p_sortListWidgetItem);
            } else if (p_connectItem->m_connectPath != p_lanItem->getConnectionPath()) {
                p_lanItem->updateConnectionPath(p_connectItem->m_connectPath);
            }
        }

    } else if (!m_activeConnectionMap.contains(newUuid)){
        if (p_connectItem->m_ifaceName == m_currentDeviceName
                || p_connectItem->m_ifaceName.isEmpty()) {
            QListWidgetItem *p_listWidgetItem = insertNewItem(p_connectItem, m_inactivatedLanListWidget);
            if (m_inactiveConnectionMap.contains(newUuid)) {
                qDebug()<<LOG_FLAG << "has contained connection uuid" << newUuid;
            }
            m_inactiveConnectionMap.insert(newUuid, p_listWidgetItem);
        }
    } else {
        qWarning() << LOG_FLAG << newUuid <<" is in activemap, so not process.";
    }

    return;
}

void LanPage::updateActiveConnectionProperty(KyConnectItem *p_connectItem)
{
    QString newUuid = p_connectItem->m_connectUuid;

    if (m_activeConnectionMap.contains(newUuid)) {
        QListWidgetItem *p_listWidgetItem = m_activeConnectionMap.value(newUuid);
        LanListItem *p_lanItem = (LanListItem *)m_activatedLanListWidget->itemWidget(p_listWidgetItem);
        if (m_currentDeviceName != p_connectItem->m_ifaceName) {
            m_activeConnectionMap.remove(newUuid);
            int takeRow = m_activatedLanListWidget->row(p_listWidgetItem);
            m_activatedLanListWidget->takeItem(takeRow);

            delete p_lanItem;
            p_lanItem = nullptr;

            p_lanItem = new LanListItem();
            m_activatedLanListWidget->addItem(p_listWidgetItem);
            m_activatedLanListWidget->setItemWidget(p_listWidgetItem, p_lanItem);
            m_activeConnectionMap.insert(EMPTY_CONNECT_UUID, p_listWidgetItem);
        } else {
            if (p_lanItem->getConnectionName() != p_connectItem->m_connectName) {
                p_lanItem->updateConnectionName(p_connectItem->m_connectName);
            }

            if (p_lanItem->getConnectionName() != p_connectItem->m_connectPath) {
                p_lanItem->updateConnectionPath(p_connectItem->m_connectPath);
            }

        }
    }

    return;
}

void LanPage::onUpdateConnection(QString uuid)
{
    if (!m_connectResourse->isWiredConnection(uuid)) {
        return;
    }

    qDebug() << "[LanPage]:Connection property Changed." << Q_FUNC_INFO << __LINE__;

    KyConnectItem *p_newItem = nullptr;
    if (m_connectResourse->isActivatedConnection(uuid)) {
        p_newItem = m_activeResourse->getActiveConnectionByUuid(uuid);
        if (nullptr == p_newItem) {
            qWarning()<<"[LanPage] get item failed, when update activate connection."
                      <<"connection uuid" << uuid;
            return;
        }

        updateActiveConnectionProperty(p_newItem);
    } else {
        p_newItem = m_connectResourse->getConnectionItemByUuid(uuid);
        if (nullptr == p_newItem) {
            qWarning()<<"[LanPage] get item failed, when update connection."
                      <<"connection uuid"<<uuid;
            return;
        }

        updateConnectionProperty(p_newItem);
    }

    sendLanUpdateSignal(p_newItem);

    delete p_newItem;
    p_newItem = nullptr;

    return;
}

void LanPage::setWiredDeviceEnable(const QString& devName, bool enable)
{
    m_deviceResource->setDeviceManaged(devName, enable);
}

void LanPage::deleteWired(const QString &connUuid)
{
    qDebug() << "[LanPage] deleteWired" << connUuid;
    if (connUuid == nullptr) {
        return;
    }
    m_wiredConnectOperation->deleteWiredConnect(connUuid);
}

bool LanPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_settingsLabel) {
        if (event->type() == QEvent::MouseButtonRelease) {
            onShowControlCenter();
        }
    }/* else if(watched == m_netSwitch){
        if (event->type() == QEvent::MouseButtonRelease) {
            qDebug()<<"[LanPage] On lan switch button clicked! Status:" <<m_netSwitch->isChecked()
                   <<"devices count:"<<m_devList.count();

            if (m_devList.count() == 0) {
                this->showDesktopNotify(tr("No ethernet device avaliable"), "networkwrong");
                m_netSwitch->setChecked(false);
                m_netSwitch->setCheckable(false);
            }
        }

    }*/ else if (watched == m_activatedLanListWidget) {
        //去掉无右键菜单显示时的选中效果
        if (event->type() ==  QEvent::FocusIn) {
            if (m_activatedLanListWidget->currentItem() != nullptr) {
                m_activatedLanListWidget->currentItem()->setSelected(false);
            }
        }
    } else if (watched == m_inactivatedLanListWidget) {
        //去掉无右键菜单显示时的选中效果
        if (event->type() == QEvent::FocusIn) {
            if (m_inactivatedLanListWidget->currentItem() != nullptr) {
                m_inactivatedLanListWidget->currentItem()->setSelected(false);
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void LanPage::onWiredEnabledChanged(bool enabled)
{
//    if (m_devList.isEmpty()) {
//        qDebug() << "[LanPage] have no device to use "  << Q_FUNC_INFO << __LINE__;
//        return;
//    }

//    if (m_netSwitch->isChecked() == enabled) {
//        return;
//    } else {
//        m_switchGsettings->set(WIRED_SWITCH, enabled);
//    }
}

void LanPage::activateWired(const QString& devName, const QString& connUuid)
{
    qDebug() << "[LanPage] activateWired" << devName << connUuid;
    if (!m_deviceResource->wiredDeviceIsCarriered(devName)) {
        qDebug() << LOG_FLAG << devName << "is not carried, so can not activate connection";
        this->showDesktopNotify(tr("Wired Device not carried"), "networkwrong");
    } else {
        m_wiredConnectOperation->activateConnection(connUuid, devName);
    }
}

void LanPage::deactivateWired(const QString& devName, const QString& connUuid)
{
    qDebug() << "[LanPage] deactivateWired" << devName << connUuid;
    QString name("");
    m_wiredConnectOperation->deactivateWiredConnection(name, connUuid);
}

void LanPage::showDetailPage(QString devName, QString uuid)
{
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
       qWarning()<<"[LanPage] GetConnectionItemByUuid is empty when showDetailPage."
                      <<"device name"<<devName
                      <<"connect uuid"<<uuid;
       return;
    }

    if (m_lanPagePtrMap.contains(p_item->m_connectPath)) {
        if (m_lanPagePtrMap[p_item->m_connectPath] != nullptr) {
            qDebug() << "[LanPage] ShowLanDetailPage" << uuid << "already create,just raise";
            KWindowSystem::activateWindow(m_lanPagePtrMap[p_item->m_connectPath]->winId());
            KWindowSystem::raiseWindow(m_lanPagePtrMap[p_item->m_connectPath]->winId());
            return;
        }
    }

    NetDetail *netDetail = new NetDetail(devName, p_item->m_connectName, uuid, isActive, false, false);
    m_lanPagePtrMap.insert(p_item->m_connectPath, netDetail);
    netDetail->show();
    KWindowSystem::raiseWindow(netDetail->winId());
    netDetail->centerToScreen();
    kdk::WindowManager::setSkipSwitcher(netDetail->windowHandle(), true);
    kdk::WindowManager::setSkipTaskBar(netDetail->windowHandle(), true);
    kdk::WindowManager::setIconName(netDetail->windowHandle(), "lingmo-network");

    connect(netDetail, &NetDetail::detailPageClose, [&](QString deviceName, QString lanName, QString lanUuid){
        if (lanUuid.isEmpty()) {
            return;
        }
        KyConnectItem *currentItem = nullptr;
        if (m_connectResourse->isActivatedConnection(lanUuid)) {
            currentItem = m_activeResourse->getActiveConnectionByUuid(lanUuid);
        } else {
            currentItem = m_connectResourse->getConnectionItemByUuid(lanUuid);
        }
        if (currentItem == nullptr) {
            return;
        }
        if (m_lanPagePtrMap.contains(currentItem->m_connectPath)) {
            m_lanPagePtrMap[currentItem->m_connectPath] = nullptr;
        }
        delete currentItem;
        currentItem = nullptr;
    });

    delete p_item;
    p_item = nullptr;
}

bool LanPage::lanIsConnected()
{
    if (m_activeResourse->wiredConnectIsActived()) {
        return true;
    } else {
        return false;
    }
}

void LanPage::getWiredDeviceConnectState(QMap<QString, QString> &map)
{
    map.clear();
    if (m_enableDeviceList.isEmpty()) {
        return;
    }

    for (const auto devname : m_enableDeviceList) {
        NetworkManager::Connectivity state;
        QList<KyConnectItem *> activedList;
        m_deviceResource->getDeviceConnectivity(devname, state);

        if (state < NetworkManager::Connectivity::Full) {
            m_activeResourse->getActiveConnectionList(devname, NetworkManager::ConnectionSettings::Wired, activedList);
            if (!activedList.isEmpty()) {
                map.insert(devname, QString(tr("Connected: ")) + activedList.at(0)->m_connectName + " " + QString(tr("(Limited)")));
            } else {
                map.insert(devname, tr("Not Connected"));
            }

        } else if (state == NetworkManager::Connectivity::Full) {
            m_activeResourse->getActiveConnectionList(devname, NetworkManager::ConnectionSettings::Wired, activedList);
            if (!activedList.isEmpty()) {
                map.insert(devname, QString(tr("Connected: ")) + activedList.at(0)->m_connectName);
            }

        } else {
            qDebug() << devname << " Network connectivity is unknown.";
        }
    }
}

void LanPage::showRate()
{
    //定时获取网速
    setNetSpeed = new QTimer(this);
    setNetSpeed->setTimerType(Qt::PreciseTimer);
    connect(setNetSpeed, &QTimer::timeout,  [&]() {
        onSetNetSpeed(m_activatedLanListWidget, m_activeConnectionMap.contains(EMPTY_SSID), m_currentDeviceName);
    });
}

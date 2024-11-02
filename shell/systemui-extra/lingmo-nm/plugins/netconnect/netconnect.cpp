/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
#include "netconnect.h"
#include "ui_netconnect.h"

#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <QtDBus>
#include <QDir>
#include <QDebug>
#include <QtAlgorithms>

#define WIRED_TYPE          0
#define ITEMHEIGH           50
#define LAN_TYPE           0
#define CONTROL_CENTER_WIFI              "org.lingmo.control-center.wifi.switch"
#define LINGMO_APP_MANAGER_NAME           "com.lingmo.AppManager"
#define LINGMO_APP_MANAGER_PATH           "/com/lingmo/AppManager"
#define LINGMO_APP_MANAGER_INTERFACE      "com.lingmo.AppManager"
const QString KLanSymbolic      = "network-wired-connected-symbolic";
const QString NoNetSymbolic     = "network-wired-disconnected-symbolic";

const QString WIRED_SWITCH = "wiredswitch";
const QByteArray GSETTINGS_SCHEMA = "org.lingmo.lingmo-nm.switch";

#define ACTIVATING   1
#define ACTIVATED    2
#define DEACTIVATING 3
#define DEACTIVATED  4

#define NO_MARGINS 0,0,0,0
#define TOP_MARGINS 0,8,0,0
#define MAIN_LAYOUT_MARGINS 0,0,0,8
#define SPACING 8
#define ICON_SIZE 16,16

bool sortByVal(const QPair<QString, int> &l, const QPair<QString, int> &r) {
    return (l.second < r.second);
}

void NetConnect::showDesktopNotify(const QString &message)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<(tr("Settings"))
       <<((unsigned int) 0)
       <<QString("gnome-dev-ethernet")
       <<tr("Settings desktop message") //显示的是什么类型的信息
       <<message //显示的具体信息
       <<QStringList()
       <<QVariantMap()
       <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}


NetConnect::NetConnect() :  mFirstLoad(true) {
    qDBusRegisterMetaType<QStringList>();
    qDBusRegisterMetaType<QList<QStringList>>();

    QTranslator* translator = new QTranslator(this);
    translator->load("/usr/share/lingmo-nm/netconnect/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    pluginName = tr("LAN");
    pluginType = NETWORK;
}

NetConnect::~NetConnect() {
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
        delete m_interface;
        delete m_switchGsettings;
    }
}

QString NetConnect::plugini18nName() {
    return pluginName;
}

int NetConnect::pluginTypes() {
    return pluginType;
}

QWidget *NetConnect::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::NetConnect;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        m_interface = new QDBusInterface("com.lingmo.network",
                                         "/com/lingmo/network",
                                         "com.lingmo.network",
                                         QDBusConnection::sessionBus());
        if(!m_interface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }

        qDBusRegisterMetaType<QVector<QStringList>>();

        initSearchText();
        initComponent();
    }
    return pluginWidget;
}

const QString NetConnect::name() const {

    return QStringLiteral("netconnect");
}

bool NetConnect::isEnable() const
{
    return true;
}


bool NetConnect::isShowOnHomePage() const
{
    return false;
}

QIcon NetConnect::icon() const
{
    return QIcon::fromTheme("network-wired-symbolic");
}

QString NetConnect::translationPath() const
{
    return "/usr/share/lingmo-nm/netconnect/%1.ts";
}

void NetConnect::initSearchText() {
    //~ contents_path /netconnect/Advanced settings"
    ui->detailBtn->setText(tr("Advanced settings"));
    ui->titleLabel->setText(tr("LAN"));
    //~ contents_path /netconnect/open
    tr("open");
    ui->openLabel->setText(tr("LAN"));
}

bool NetConnect::eventFilter(QObject *w, QEvent *e) {
    if (e->type() == QEvent::Enter) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(button);border-radius:4px;}");
    } else if (e->type() == QEvent::Leave) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(base);border-radius:4px;}");
    }

//    if (w == wiredSwitch) {
//        if (e->type() == QMouseEvent::MouseButtonRelease) {
//            if (!wiredSwitch->isCheckable()) {
//                showDesktopNotify(tr("No ethernet device avaliable"));
//            } else {
//                UkccCommon::buriedSettings(QString("netconnect"), QString("Open"), QString("settings"),wiredSwitch->isChecked()?"false":"true");
//                if (m_interface != nullptr && m_interface->isValid()) {
//                    m_interface->call(QStringLiteral("setWiredSwitchEnable"), !wiredSwitch->isChecked());
//                }
//                return true;
//            }
//        }
//    }

    return QObject::eventFilter(w,e);
}

void NetConnect::initComponent() {
//    wiredSwitch = new KSwitchButton(pluginWidget);
//    ui->openWIifLayout->addWidget(wiredSwitch);
    ui->openWIifLayout->setContentsMargins(0,0,8,0);
    ui->openWifiFrame->hide();
    ui->detailLayOut->setContentsMargins(MAIN_LAYOUT_MARGINS);
    ui->verticalLayout_3->setContentsMargins(NO_MARGINS);
    ui->verticalLayout_3->setSpacing(8);
    ui->availableLayout->setSpacing(SPACING);
    ui->horizontalLayout->setContentsMargins(TOP_MARGINS);

//    wiredSwitch->installEventFilter(this);

//    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
//        m_switchGsettings = new QGSettings(GSETTINGS_SCHEMA);

//        setSwitchStatus();

//        connect(m_switchGsettings, &QGSettings::changed, this, [=] (const QString &key) {
//            if (key == WIRED_SWITCH) {
//                setSwitchStatus();
//            }
//        });
//    } else {
//        wiredSwitch->blockSignals(true);
//        wiredSwitch->setChecked(true);
//        wiredSwitch->blockSignals(false);
//        qDebug()<<"[Netconnect] org.lingmo.lingmo-nm.switch is not installed!";
//    }

    getDeviceStatusMap(deviceStatusMap);
//    if (deviceStatusMap.isEmpty()) {
//        qDebug() << "[Netconnect] no device exist when init, set switch disable";
//        wiredSwitch->setChecked(false);
//        wiredSwitch->setCheckable(false);
//    }
    initNet();

    if (/*!wiredSwitch->isChecked() || */deviceStatusMap.isEmpty() || !m_interface->isValid()) {
        hideLayout(ui->availableLayout);
    }


    // 有线网络断开或连接时刷新可用网络列表
    connect(m_interface, SIGNAL(lanActiveConnectionStateChanged(QString, QString, int)), this, SLOT(onActiveConnectionChanged(QString, QString, int)), Qt::QueuedConnection);
    //有线网络新增时添加网络
    connect(m_interface, SIGNAL(lanAdd(QString, QStringList)), this, SLOT(onLanAdd(QString, QStringList)), Qt::QueuedConnection);
    //删除有线网络
    connect(m_interface, SIGNAL(lanRemove(QString)), this, SLOT(onLanRemove(QString)), Qt::QueuedConnection);
    //更新有线网络
    connect(m_interface, SIGNAL(lanUpdate(QString, QStringList)), this, SLOT(updateLanInfo(QString, QStringList)),Qt::QueuedConnection);
    //网卡插拔处理
    connect(m_interface, SIGNAL(deviceStatusChanged()), this, SLOT(onDeviceStatusChanged()),Qt::QueuedConnection);
    //网卡name处理
    connect(m_interface, SIGNAL(deviceNameChanged(QString, QString, int)), this, SLOT(onDeviceNameChanged(QString, QString, int)),Qt::QueuedConnection);


    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        UkccCommon::buriedSettings(QString("netconnect"), QString("Advanced settings"), QString("clicked"));
        runExternalApp();
    });
}

//获取网卡列表
void NetConnect::getDeviceStatusMap(QMap<QString, bool> &map)
{
    if (m_interface == nullptr || !m_interface->isValid()) {
        return;
    }
    map.clear();
    qDebug() << "[NetConnect]call getDeviceListAndEnabled"  << __LINE__;
    QDBusReply<QVariantMap> reply = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),0);
    qDebug() << "[NetConnect]call getDeviceListAndEnabled Respond"  << __LINE__;
    if(!reply.isValid())
    {
        qWarning() << "[NetConnect]getWiredDeviceList error:" << reply.error().message();
        return;
    }

    QVariantMap::const_iterator item = reply.value().cbegin();
    while (item != reply.value().cend()) {
        map.insert(item.key(), item.value().toBool());
        item ++;
    }
}

//lanUpdate
void NetConnect::updateLanInfo(QString deviceName, QStringList lanInfo)
{
    //she bei gui shu bian hua && you xian ming cheng bian hua
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (deviceName.isEmpty()) {
            //变为无指定网卡，所有列表都要添加
            if (!iter.value()->itemMap.contains(lanInfo.at(1))) {
                qDebug() << "[NetConnect]" << lanInfo.at(0) << " change to device none, add every list";
                addOneLanFrame(iter.value(), iter.key(), lanInfo);
            } else {
                if (iter.value()->itemMap[lanInfo.at(1)]->titileLabel->text() != lanInfo.at(0)) {
                    qDebug() << "[NetConnect]" << iter.key()
                             << iter.value()->itemMap[lanInfo.at(1)]->titileLabel->text() << "change to" << lanInfo.at(0);
                    iter.value()->itemMap[lanInfo.at(1)]->titileLabel->setText(lanInfo.at(0));
                }
            }
        } else {
            if (iter.key() != deviceName) {
                qDebug() << "[NetConnect]" << lanInfo.at(0) << " not belongs to " << iter.key();
                removeOneLanFrame(iter.value(), deviceName, lanInfo.at(1));
            } else {
                if (!iter.value()->itemMap.contains(lanInfo.at(1))) {
                    qDebug() << "[NetConnect]" << lanInfo.at(0) << " now belongs to " << deviceName;
                    addOneLanFrame(iter.value(), deviceName, lanInfo);
                } else {
                    qDebug() << "[NetConnect]" << deviceName
                             << iter.value()->itemMap[lanInfo.at(1)]->titileLabel->text() << "change to" << lanInfo.at(0);
                    if (iter.value()->itemMap[lanInfo.at(1)]->titileLabel->text() != lanInfo.at(0)) {
                        iter.value()->itemMap[lanInfo.at(1)]->titileLabel->setText(lanInfo.at(0));
                    }
                }
            }
        }
    }
}


//总开关
void NetConnect::setSwitchStatus()
{
//    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
//        bool status = m_switchGsettings->get(WIRED_SWITCH).toBool();
//        wiredSwitch->blockSignals(true);
//        wiredSwitch->setChecked(status);
//        wiredSwitch->blockSignals(false);
//        if (!status) {
//            hideLayout(ui->availableLayout);
//        } else {
//            showLayout(ui->availableLayout);
//        }
//    } else {
//        qDebug()<<"[netconnect] org.lingmo.lingmo-nm.switch is not installed!";
//    }

}

//总体隐藏
void NetConnect::hideLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i)
    {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        itemFrame->hide();
    }
}

//总体显示
void NetConnect::showLayout(QVBoxLayout * layout) {
    for (int i = layout->layout()->count()-1; i >= 0; --i)
    {
        QLayoutItem *it = layout->layout()->itemAt(i);
        ItemFrame *itemFrame = qobject_cast<ItemFrame *>(it->widget());
        itemFrame->show();
    }
}

//初始化
void NetConnect::initNet()
{
    //先构建每个设备的列表头
    QStringList deviceList = deviceStatusMap.keys();
    for (int i = 0; i < deviceList.size(); ++i) {
        addDeviceFrame(deviceList.at(i));
    }
    //再填充每个设备的列表
    for (int i = 0; i < deviceList.size(); ++i) {
        initNetListFromDevice(deviceList.at(i));
    }
}

//刪除
void NetConnect::deleteOneLan(QString ssid, int type)
{
    qDebug() << "[NetConnect]call deleteConnect" << __LINE__;
    m_interface->call(QStringLiteral("deleteConnect"), type, ssid);
    qDebug() << "[NetConnect]call deleteConnect respond" << __LINE__;
}

void NetConnect::runExternalApp() {
    if (!LaunchApp("nm-connection-editor.desktop")){
        QString cmd = "nm-connection-editor";
        QProcess process(this);
        process.startDetached(cmd);
    }
}

//激活
void NetConnect::activeConnect(QString ssid, QString deviceName, int type) {
    if (m_interface == nullptr || !m_interface->isValid()) {
        return;
    }
    qDebug() << "[NetConnect]call activateConnect" << __LINE__;
    m_interface->call(QStringLiteral("activateConnect"),type, deviceName, ssid);
    qDebug() << "[NetConnect]call activateConnect respond" << __LINE__;
}

//断开
void NetConnect::deActiveConnect(QString ssid, QString deviceName, int type) {
    if (m_interface == nullptr || !m_interface->isValid()) {
        return;
    }
    qDebug() << "[NetConnect]call deActivateConnect" << __LINE__;
    m_interface->call(QStringLiteral("deActivateConnect"),type, deviceName, ssid);
    qDebug() << "[NetConnect]call deActivateConnect respond" << __LINE__;
}

//初始化设备列表 网卡标号问题？
void NetConnect::initNetListFromDevice(QString deviceName)
{
    qDebug() << "[NetConnect]initNetListFromDevice " << deviceName;
    if (!deviceFrameMap.contains(deviceName)) {
        qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " not exist";
        return;
    }
    if (m_interface == nullptr || !m_interface->isValid()) {
        return;
    }

    QMap<QString, QList<QStringList>> variantList = getWiredList();
    if (variantList.size() == 0) {
        qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " list empty";
        return;
    }

    QMap<QString, QList<QStringList>>::iterator iter;
    for (iter = variantList.begin(); iter != variantList.end(); iter++) {
        if (deviceName == iter.key()) {
            QList<QStringList> wlanListInfo = iter.value();
            //处理列表 已连接
            qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " acitved lan " << wlanListInfo.at(0);
            addLanItem(deviceFrameMap[deviceName], deviceName,  wlanListInfo.at(0), true);
            //处理列表 未连接
            for (int i = 1; i < wlanListInfo.length(); i++) {
                qDebug() << "[NetConnect]initNetListFromDevice " << deviceName << " deacitved lan " << wlanListInfo.at(i);
                addLanItem(deviceFrameMap[deviceName], deviceName, wlanListInfo.at(i), false);
            }
        }
    }
    return;
}

//初始化时添加一个项 不考虑顺序
void NetConnect::addLanItem(ItemFrame *frame, QString devName, QStringList infoList, bool isActived)
{
    if (frame == nullptr) {
        return;
    }
    if (infoList.size() == 1) {
        return;
    }

    LanItem * lanItem = new LanItem(isActived, pluginWidget);
    QString iconPath = KLanSymbolic;
    if (isActived) {
        lanItem->statusLabel->setText(tr("connected"));
    } else {
        lanItem->statusLabel->setText(tr("not connected"));
    }
    QIcon searchIcon = QIcon::fromTheme(iconPath);
//    if (iconPath != KLanSymbolic && iconPath != NoNetSymbolic) {
//        lanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
//    }
    lanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(ICON_SIZE))));
    lanItem->titileLabel->setText(infoList.at(0));

    lanItem->uuid = infoList.at(1);
    lanItem->dbusPath = infoList.at(2);

    connect(lanItem->infoLabel, &GrayInfoButton::clicked, this, [=]{
        // open landetail page
        if (m_interface == nullptr || !m_interface->isValid()) {
            return;
        }
        UkccCommon::buriedSettings(QString("netconnect"), QString("info"), QString("clicked"));
        qDebug() << "[NetConnect]call showPropertyWidget" << __LINE__;
        m_interface->call(QStringLiteral("showPropertyWidget"), devName, infoList.at(1));
        qDebug() << "[NetConnect]call showPropertyWidget respond" << __LINE__;
    });

    lanItem->isAcitve = isActived;
    lanItem->setConnectActionText(lanItem->isAcitve);

    connect(lanItem, &QPushButton::clicked, this, [=] {
        if (lanItem->isAcitve || lanItem->loading) {
            deActiveConnect(lanItem->uuid, devName, WIRED_TYPE);
        } else {
            activeConnect(lanItem->uuid, devName, WIRED_TYPE);
        }
    });

    connect(lanItem, &LanItem::connectActionTriggered, this, [=] {
        activeConnect(lanItem->uuid, devName, WIRED_TYPE);
    });
    connect(lanItem, &LanItem::disconnectActionTriggered, this, [=] {
        deActiveConnect(lanItem->uuid, devName, WIRED_TYPE);
    });
    connect(lanItem, &LanItem::deleteActionTriggered, this, [=] {
        deleteOneLan(lanItem->uuid, WIRED_TYPE);
    });

    //记录到deviceFrame的itemMap中
    deviceFrameMap[devName]->itemMap.insert(infoList.at(1), lanItem);
    qDebug()<<"insert " << infoList.at(1) << " to " << devName << " list";
    frame->lanItemLayout->addWidget(lanItem);
}

//增加设备
void NetConnect::addDeviceFrame(QString devName)
{
    if (m_interface == nullptr || !m_interface->isValid()) {
        return;
    }
    qDebug() << "[NetConnect]addDeviceFrame " << devName;

    qDebug() << "[NetConnect]call getDeviceListAndEnabled"  << __LINE__;
    QDBusReply<QVariantMap> reply = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),0);
    qDebug() << "[NetConnect]call getDeviceListAndEnabled Respond"  << __LINE__;
    if(!reply.isValid())    {
        qWarning() << "[NetConnect]getWiredDeviceList error:" << reply.error().message();
        return;
    }
    QMap<QString,bool> map;
    QVariantMap::const_iterator item = reply.value().cbegin();
    while (item != reply.value().cend()) {
        map.insert(item.key(), item.value().toBool());
        item ++;
    }

    bool enable = true;
    if (map.contains(devName)) {
        enable = map[devName];
    }


    ItemFrame *itemFrame = new ItemFrame(devName, pluginWidget);
    ui->availableLayout->addWidget(itemFrame);
    itemFrame->deviceFrame->deviceLabel->setText(tr("card")+/*QString("%1").arg(count)+*/"："+devName);
//    itemFrame->deviceFrame->deviceSwitch->setChecked(enable);
    if (enable) {
        itemFrame->lanItemFrame->show();
        itemFrame->deviceFrame->dropDownLabel->show();
    } else {
        itemFrame->lanItemFrame->hide();
        itemFrame->deviceFrame->dropDownLabel->hide();
        itemFrame->deviceFrame->dropDownLabel->setDropDownStatus(false);
    }
    deviceFrameMap.insert(devName, itemFrame);
    qDebug() << "[NetConnect]deviceFrameMap insert" << devName;

//    connect(itemFrame->deviceFrame, &DeviceFrame::deviceSwitchClicked ,this, [=] (bool checked) {
//        UkccCommon::buriedSettings(QString("netconnect"), "device open", QString("settings"), checked?"true":"fasle");
//        qDebug() << "[NetConnect]call setDeviceEnable" << devName << checked << __LINE__;
//        m_interface->call(QStringLiteral("setDeviceEnable"), devName, checked);
//        qDebug() << "[NetConnect]call setDeviceEnable Respond"  << __LINE__;
//    });

//    connect(itemFrame->deviceFrame->deviceSwitch, &KSwitchButton::stateChanged, this, [=] (bool checked) {

//        if (checked) {
//            qDebug() << "[NetConnect]set " << devName << "status" << true;
//            itemFrame->lanItemFrame->show();
//            itemFrame->deviceFrame->dropDownLabel->show();
//            itemFrame->addLanWidget->show();
//            itemFrame->deviceFrame->dropDownLabel->setDropDownStatus(true);
//            deviceStatusMap[devName] = true;
//        } else {
//            qDebug() << "[NetConnect]set " << devName << "status" << false;
//            itemFrame->lanItemFrame->hide();
//            itemFrame->deviceFrame->dropDownLabel->hide();
//            itemFrame->addLanWidget->hide();
//            deviceStatusMap[devName] = false;
//        }
//    });

    connect(itemFrame->addLanWidget, &AddNetBtn::clicked, this, [=](){
        UkccCommon::buriedSettings(pluginName, "Add net", QString("clicked"));
        if (m_interface != nullptr && m_interface->isValid()) {
            qDebug() << "[NetConnect]call showCreateWiredConnectWidget" << devName  << __LINE__;
            m_interface->call(QStringLiteral("showCreateWiredConnectWidget"), devName);
            qDebug() << "[NetConnect]call setDeviceEnable Respond"  << __LINE__;
        }
    });
}

//减少设备
void NetConnect::removeDeviceFrame(QString devName)
{
    qDebug() << "[NetConnect]removeDeviceFrame " << devName;
    if (deviceFrameMap.contains(devName)) {
        ItemFrame *item = deviceFrameMap[devName];
        if (item->lanItemFrame->layout() != NULL) {
            QLayoutItem* layoutItem;
            while ((layoutItem = item->lanItemFrame->layout()->takeAt(0)) != NULL) {
                delete layoutItem->widget();
                delete layoutItem;
                layoutItem = nullptr;
            }
            item->itemMap.clear();
        }
        delete item;
        item = nullptr;
        deviceFrameMap.remove(devName);
        qDebug() << "[NetConnect]deviceFrameMap remove" << devName;
    }
}

//device add or remove=================================
void NetConnect::onDeviceStatusChanged()
{
    qDebug()<<"[NetConnect]onDeviceStatusChanged";
    QEventLoop eventloop;
    QTimer::singleShot(300, &eventloop, SLOT(quit()));
    eventloop.exec();
    QStringList list;
    QMap<QString, bool> map;
    getDeviceStatusMap(map);
    list = map.keys();

    QStringList removeList;
    QMap<QString, bool> addMap;

    //remove的设备
    for (int i = 0; i< deviceStatusMap.keys().size(); ++i) {
        if (!list.contains(deviceStatusMap.keys().at(i))) {
            qDebug() << "[NetConnect]onDeviceStatusChanged " << deviceStatusMap.keys().at(i) << "was removed";
            removeList << deviceStatusMap.keys().at(i);
        }
    }

    //add的设备
    for (int i = 0; i< list.size(); ++i) {
        if (!deviceStatusMap.keys().contains(list.at(i))) {
            qDebug() << "[NetConnect]onDeviceStatusChanged " << list.at(i) << "was add, init status" << map[list.at(i)];
            addMap.insert(list.at(i),map[list.at(i)]);
        }
    }

    for (int i = 0; i < removeList.size(); ++i) {
        removeDeviceFrame(removeList.at(i));
    }

    QStringList addList = addMap.keys();
    for (int i = 0; i < addList.size(); ++i) {
        qDebug() << "add a device " << addList.at(i) << "status" << map[addList.at(i)];
        addDeviceFrame(addList.at(i));
        initNetListFromDevice(addList.at(i));
    }
    deviceStatusMap = map;
//    if (deviceStatusMap.isEmpty()) {
//        wiredSwitch->setChecked(false);
//        wiredSwitch->setCheckable(false);
//    } else {
//        wiredSwitch->setCheckable(true);
//        setSwitchStatus();
//    }

//    QMap<QString, ItemFrame *>::iterator iter;
//    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
//        if (deviceStatusMap.contains(iter.key())) {
//            if (iter.value()->deviceFrame->deviceSwitch->isChecked() != deviceStatusMap[iter.key()]) {
//                iter.value()->deviceFrame->deviceSwitch->setChecked(deviceStatusMap[iter.key()]);
//            }
//        }
//    }
}

void NetConnect::onDeviceNameChanged(QString oldName, QString newName, int type)
{
    if (WIRED_TYPE != type || !deviceFrameMap.contains(oldName) || !deviceStatusMap.contains(oldName)) {
        qDebug() << "[NetConnect]onDeviceNameChanged no such device " << oldName;
        return;
    }

    if (deviceFrameMap.contains(newName) && deviceStatusMap.contains(newName)) {
        qDebug() << "[NetConnect]onDeviceNameChanged already has device " << newName;
        return;
    }

    qDebug() << "[NetConnect]onDeviceNameChanged " << oldName << "change to" << newName;

    //shan chu chong jian
    removeDeviceFrame(oldName);
    removeDeviceFrame(newName);

    getDeviceStatusMap(deviceStatusMap);
    if (deviceStatusMap.contains(newName)) {
        addDeviceFrame(newName);
        initNetListFromDevice(newName);
    }
}

//wifi add===============================================================
void NetConnect::onLanAdd(QString deviceName, QStringList lanInfo)
{
    qDebug()<<"[NetConnect]onLanAdd "<< deviceName << " " << lanInfo;

    if (!deviceName.isEmpty() && !deviceStatusMap.contains(deviceName)) {
        return;
    }

    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        if (deviceName.isEmpty()) {
                qDebug() << "[NetConnect]onLanAdd every list" << iter.key();
                addOneLanFrame(iter.value(), iter.key(), lanInfo);
        } else if (deviceName == iter.key()) {
                qDebug() << "[NetConnect]onLanAdd "<< deviceName;
                addOneLanFrame(iter.value(), deviceName, lanInfo);
                break;
        }
    }
}

//wifi remove =============================================================
void NetConnect::onLanRemove(QString lanPath)
{
    //开关已关闭 忽略
//    if (!wifiSwtch->isChecked()) {
//        qDebug() << "[NetConnect]recieve network remove,but wireless switch is off";
//        return;
//    }

    qDebug()<<"[NetConnect]lan remove " << "dbus path:" << lanPath;
    QMap<QString, ItemFrame *>::iterator iter;
    for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
        QMap<QString, LanItem *>::iterator itemIter;
        for (itemIter = iter.value()->itemMap.begin(); itemIter != iter.value()->itemMap.end(); itemIter++) {
            if (itemIter.value()->dbusPath == lanPath) {
               qDebug()<<"[NetConnect]lan remove " << lanPath << " find in " << itemIter.value()->titileLabel->text();
               QString key = itemIter.key();
               iter.value()->lanItemLayout->removeWidget(itemIter.value());
               delete itemIter.value();
               iter.value()->itemMap.remove(key);
               break;
            }
        }
    }
}

//增加一项
void NetConnect::addOneLanFrame(ItemFrame *frame, QString deviceName, QStringList infoList)
{
    if (nullptr == frame) {
        return;
    }

    if (frame->itemMap.contains(infoList.at(1))) {
            qDebug() << "[NetConnect]Already exist a lan " << infoList.at(1) << " in " << deviceName;
            return;
    }

    qDebug() << "[NetConnect]addOneLanFrame" << deviceName << infoList.at(0);
    QString connName = infoList.at(0);
    QString connUuid = infoList.at(1);
    QString connDbusPath = infoList.at(2);
    LanItem * lanItem = new LanItem(pluginWidget);

    QString iconPath;
    iconPath = KLanSymbolic;
    lanItem->statusLabel->setText(tr("not connected"));

    QIcon searchIcon = QIcon::fromTheme(iconPath);
//    if (iconPath != KLanSymbolic && iconPath != NoNetSymbolic) {
//        lanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
//    }
    lanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(ICON_SIZE))));
    lanItem->titileLabel->setText(connName);

    lanItem->uuid = connUuid;
    lanItem->dbusPath = connDbusPath;

    connect(lanItem->infoLabel, &GrayInfoButton::clicked, this, [=]{
        // open landetail page
        if (m_interface == nullptr || !m_interface->isValid()) {
            return;
        }
        UkccCommon::buriedSettings(QString("netconnect"), QString("info"), QString("clicked"));
        qDebug() << "[NetConnect]call showPropertyWidget" << deviceName << connUuid << __LINE__;
        m_interface->call(QStringLiteral("showPropertyWidget"), deviceName, connUuid);
        qDebug() << "[NetConnect]call showPropertyWidget respond" << __LINE__;
    });

    lanItem->isAcitve = false;
    lanItem->setConnectActionText(lanItem->isAcitve);

    connect(lanItem, &QPushButton::clicked, this, [=] {
        if (lanItem->isAcitve || lanItem->loading) {
            deActiveConnect(lanItem->uuid, deviceName, WIRED_TYPE);
        } else {
            activeConnect(lanItem->uuid, deviceName, WIRED_TYPE);
        }
    });

    connect(lanItem, &LanItem::connectActionTriggered, this, [=] {
        activeConnect(lanItem->uuid, deviceName, WIRED_TYPE);
    });
    connect(lanItem, &LanItem::disconnectActionTriggered, this, [=] {
        deActiveConnect(lanItem->uuid, deviceName, WIRED_TYPE);
    });
    connect(lanItem, &LanItem::deleteActionTriggered, this, [=] {
        deleteOneLan(lanItem->uuid, WIRED_TYPE);
    });

    //记录到deviceFrame的itemMap中
    deviceFrameMap[deviceName]->itemMap.insert(connUuid, lanItem);
    int index = getInsertPos(connName, deviceName);
    qDebug()<<"[NetConnect]addOneLanFrame " << connName << " to " << deviceName << " list at pos:" << index;
    frame->lanItemLayout->insertWidget(index, lanItem);
}

void NetConnect::removeOneLanFrame(ItemFrame *frame, QString deviceName, QString uuid)
{
    if (nullptr == frame) {
        return;
    }

    if (!frame->itemMap.contains(uuid)) {
            qDebug() << "[NetConnect]not exist a lan " << uuid << " in " << deviceName;
            return;
    }

   qDebug()<<"[NetConnect]removeOneLanFrame " << uuid << " find in " << deviceName;

   frame->lanItemLayout->removeWidget(frame->itemMap[uuid]);
   delete frame->itemMap[uuid];
   frame->itemMap.remove(uuid);
}

//activeconnect status change
void NetConnect::onActiveConnectionChanged(QString deviceName, QString uuid, int status)
{
    if (uuid.isEmpty()) {
        qDebug() << "[NetConnect]onActiveConnectionChanged but uuid is empty";
        return;
    }
    qDebug() << "[NetConnect]onActiveConnectionChanged " << deviceName << uuid << status;
    LanItem * item= nullptr;
    if (deviceName.isEmpty()) {
        if (status != DEACTIVATED) {
            return;
        }
        //断开时 设备为空 说明此有线未指定设备 添加到所有列表中
        QStringList infoList;
        QMap<QString, ItemFrame *>::iterator iters;
        for (iters = deviceFrameMap.begin(); iters != deviceFrameMap.end(); iters++) {
            if (iters.value()->itemMap.contains(uuid)) {
                item = iters.value()->itemMap[uuid];
                infoList << item->titileLabel->text() << item->uuid << item->dbusPath;
                //为断开则重新插入
                int index = getInsertPos(item->titileLabel->text(), iters.key());
                qDebug() << "[NetConnect]reinsert" << item->titileLabel->text() << "pos" << index << "in" << iters.key() << "because status changes to deactive";
                deviceFrameMap[iters.key()]->lanItemLayout->removeWidget(item);
                deviceFrameMap[iters.key()]->lanItemLayout->insertWidget(index,item);
                itemActiveConnectionStatusChanged(item, status);
            }
        }
        //添加到所有列表中
        if (!infoList.isEmpty()) {
            QMap<QString, ItemFrame *>::iterator iter;
            for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
                if (!iter.value()->itemMap.contains(uuid)) {
                    addOneLanFrame(iter.value(), iter.key(), infoList);
                }
            }
        }
    } else {
        if (deviceFrameMap.contains(deviceName)) {
            if (deviceFrameMap[deviceName]->itemMap.contains(uuid)) {
                item = deviceFrameMap[deviceName]->itemMap[uuid];
                if (status == ACTIVATED) {
                    //为已连接则放到第一个
                    deviceFrameMap[deviceName]->lanItemLayout->removeWidget(item);
                    deviceFrameMap[deviceName]->lanItemLayout->insertWidget(0,item);
                } else if (status == DEACTIVATED) {
                    //为断开则重新插入
                    int index = getInsertPos(item->titileLabel->text(), deviceName);
                    qDebug() << "[NetConnect]reinsert" << item->titileLabel->text() << "pos" << index << "in" << deviceName << "because status changes to deactive";
                    deviceFrameMap[deviceName]->lanItemLayout->removeWidget(item);
                    deviceFrameMap[deviceName]->lanItemLayout->insertWidget(index,item);
                }
                itemActiveConnectionStatusChanged(item, status);
            }
        } else {
            if (status == ACTIVATED || status == DEACTIVATED) {
                //虚拟网卡处理
                QMap<QString, ItemFrame *>::iterator iters;
                for (iters = deviceFrameMap.begin(); iters != deviceFrameMap.end(); iters++) {
                    if (iters.value()->itemMap.contains(uuid)) {
                        removeOneLanFrame(iters.value(), iters.key(), uuid);
                     }
                }
            }
        }
    }
}

void NetConnect::itemActiveConnectionStatusChanged(LanItem *item, int status)
{
//    QString iconPath = NoNetSymbolic;
    if (status == ACTIVATING) {
        item->startLoading();
    } else if (status == ACTIVATED) {
        item->stopLoading();
//        iconPath = KLanSymbolic;
        item->statusLabel->clear();
        item->statusLabel->setMinimumSize(36,36);
        item->statusLabel->setMaximumSize(16777215,16777215);
        item->statusLabel->setText(tr("connected"));
        item->isAcitve = true;
    } else if (status == DEACTIVATING) {
        item->startLoading();
    } else {
        item->stopLoading();
        item->statusLabel->setMinimumSize(36,36);
        item->statusLabel->setMaximumSize(16777215,16777215);
        item->statusLabel->clear();
        item->isAcitve = false;
        item->statusLabel->setText(tr("not connected"));
    }
    item->setConnectActionText(item->isAcitve);
//    QIcon searchIcon = QIcon::fromTheme(iconPath);
//    item->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
}

int NetConnect::getInsertPos(QString connName, QString deviceName)
{
    qDebug() << "[NetConnect]getInsertPos" << connName << deviceName;
    int index = 0;
    if(m_interface == nullptr || !m_interface->isValid()) {
        index = 0;
    } else {

        QMap<QString, QList<QStringList>> variantList = getWiredList();

        if (!variantList.contains(deviceName)) {
            qDebug() << "[NetConnect] getInsertPos but " << deviceName << "not exist";
            return 0;
        }
        for (int i = 0; i < variantList[deviceName].size(); ++i ) {
            if (variantList[deviceName].at(i).at(0) == connName) {
                qDebug() << "pos in lingmo-nm is " << i;
                index = i;
                break;
            }
        }
        if (variantList[deviceName].at(0).size() == 1) {
            index--;
        }
    }
    return index;
}

bool NetConnect::LaunchApp(QString desktopFile)
{
    QDBusInterface m_appManagerDbusInterface(LINGMO_APP_MANAGER_NAME,
                                             LINGMO_APP_MANAGER_PATH,
                                             LINGMO_APP_MANAGER_INTERFACE,
                                             QDBusConnection::sessionBus());//局部变量

    if (!m_appManagerDbusInterface.isValid()) {
        qWarning()<<"m_appManagerDbusInterface init error";
        return false;
    } else {
        QDBusReply<bool> reply =m_appManagerDbusInterface.call("LaunchApp",desktopFile);
        return reply;
    }
}

QMap<QString, QList<QStringList>> NetConnect::getWiredList()
{
    QMap<QString, QList<QStringList>> map;

    QMap<QString, bool> statusMap;
    getDeviceStatusMap(statusMap);

    for (int i = 0; i < statusMap.keys().size(); ++i) {
        qDebug() << "[NetConnect]call getWiredList"  << __LINE__;
        QDBusReply<QVariantList> reply = m_interface->call(QStringLiteral("getWiredList"), statusMap.keys().at(i));
        qDebug() << "[NetConnect]call getWiredList respond"  << __LINE__;
        if(!reply.isValid())
        {
            qWarning() << "getWiredList error:" << reply.error().message();
            break;
        }

        QList<QStringList> list;
        for (int j = 0; j < reply.value().size(); ++j) {
            list << reply.value().at(j).toStringList();
        }
        map.insert(statusMap.keys().at(i), list);
    }
    return map;
}

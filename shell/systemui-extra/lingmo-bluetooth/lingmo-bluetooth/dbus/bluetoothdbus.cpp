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

#include "bluetoothdbus.h"
#include <QDBusMessage>

BluetoothDbus::BluetoothDbus(QObject *parent)
{
    Q_UNUSED(parent);
    KyDebug();
    userName = QString(qgetenv("USER").toStdString().data());
    adapterList = new QMap<QString, QMap<QString, QVariant>>();
    devicesList = new QMap<QString, QMap<QString, QVariant>>();
    pairedList = new QMap<QString, QMap<QString, QVariant>>();
    connectDBusSignals();
    InitAdaptersAttr();
    InitDefaultAdapterDevicesAttr();
    registerClient();
    sessDbusInterface = new SessionDbusInterface(&defaultAdapterAttr,adapterList,pairedList,this);
    connect(sessDbusInterface, &SessionDbusInterface::setLeaveLockSignal, this, &BluetoothDbus::setLeaveLock);

    leaveLockon = QGSettings(GSETTING_SCHEMA_LINGMOBLUETOOH).get("leave-lock").toBool();
    lockDev = QGSettings(GSETTING_SCHEMA_LINGMOBLUETOOH).get("leavelock-dev").toString();
    sessDbusInterface->setLeaveLockOn(leaveLockon);
    sessDbusInterface->setLeaveLockDev(lockDev);

    QDBusMessage message = QDBusMessage::createMethodCall("org.lingmo.ScreenSaver",
                                                          "/",
                                                          "org.lingmo.ScreenSaver",
                                                          "GetLockState");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        bool ret = response.arguments().takeFirst().toBool();
        if (leaveLockon && !ret)
            if (lockDev != "")
                leaveLock(lockDev, leaveLockon);
    }

    KyDebug();
}

BluetoothDbus::~BluetoothDbus()
{
    unregisterClient();
}

void BluetoothDbus::connectDBusSignals() {
   KyDebug();
    TRANSFER_DBUS_SIGNAL("showTrayWidgetUISignal", showTrayWidgetUISignal());
    TRANSFER_DBUS_SIGNAL("sendTransferFilesMesg", sendTransferFilesMesgSignal(QStringList));
    TRANSFER_DBUS_SIGNAL("sendTransferDeviceMesg", sendTransferDeviceMesgSignal(QString));

    CONNECT_SYSTEM_DBUS_SIGNAL("adapterAddSignal", adapterAddSignalSLot(QMap<QString, QVariant>));
    CONNECT_SYSTEM_DBUS_SIGNAL("adapterAttrChanged", adapterAttrChangedSLot(QString, QMap<QString, QVariant>));
    CONNECT_SYSTEM_DBUS_SIGNAL("adapterRemoveSignal", adapterRemoveSignalSLot(QString));
    CONNECT_SYSTEM_DBUS_SIGNAL("deviceAddSignal", deviceAddSLot(QMap<QString, QVariant>));
    CONNECT_SYSTEM_DBUS_SIGNAL("deviceAttrChanged", deviceAttrChangedSLot(QString, QMap<QString, QVariant>));
    CONNECT_SYSTEM_DBUS_SIGNAL("deviceRemoveSignal", devRemoveSignalSLot(QString, QMap<QString, QVariant>));
    TRANSFER_SYSTEM_DBUS_SIGNAL("ActiveConnection", activeConnectionSignal(QString,QString,QString,int,int));
    CONNECT_SYSTEM_DBUS_SIGNAL("updateClient", updateClientSLot());
    TRANSFER_SYSTEM_DBUS_SIGNAL("fileStatusChanged", statusChangedSignal(QMap<QString, QVariant>));
    CONNECT_SYSTEM_DBUS_SIGNAL("fileReceiveSignal", receiveFilesSlot(QMap<QString, QVariant>));
    CONNECT_SYSTEM_DBUS_SIGNAL("startPair", devPairSignalSLot(QMap<QString, QVariant>));
    CONNECT_SYSTEM_DBUS_SIGNAL("pingTimeSignal", pingTimeSignalSLot(QByteArray));

    QDBusConnection::systemBus().connect(SYSTEM_ACTIVE_USER_DBUS,
                                         SYSTEM_ACTIVE_USER_PATH,
                                         SYSTEM_ACTIVE_USER_INTERFACE,
                                         "ActiveUserChange",
                                         this,SIGNAL(activeUserChangedSignal(QString)));

    QDBusConnection::sessionBus().connect(MEDIA_BATTERY_SERVICE,
                                          MEDIA_BATTERY_PATH,
                                          MEDIA_BATTERY_INTERFACE,
                                          "batteryChanged",
                                          this,
                                          SLOT(devBatteryChangedSignalSlot(QString, int)));

    QDBusConnection::sessionBus().connect("org.lingmo.ScreenSaver",
                                          "/",
                                          "org.lingmo.ScreenSaver",
                                          "lock",
                                          this,
                                          SLOT(lockSlot()));

    QDBusConnection::sessionBus().connect("org.lingmo.ScreenSaver",
                                          "/",
                                          "org.lingmo.ScreenSaver",
                                          "unlock",
                                          this,
                                          SLOT(unlockSlot()));

   KyDebug();
}

bool BluetoothDbus::registerClient() {
    QMap<QString, QVariant> rcParam;
    rcParam.insert("dbusid", QVariant(QDBusConnection::systemBus().baseService()));
    rcParam.insert("username", QVariant(userName));
    rcParam.insert("type", QVariant(1));
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("registerClient");
    dbusMsg << rcParam;
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        rcParam = response.arguments().takeFirst().toMap();
        envPC = rcParam.value("envPC").toInt();
        return rcParam.value("result").toBool();
    } else
        return false;
}

bool BluetoothDbus::unregisterClient() {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("unregisterClient");
    dbusMsg << QDBusConnection::systemBus().baseService();
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        return response.arguments().takeFirst().toBool();
    } else
        return false;
}

void BluetoothDbus::updateClientSLot() {
    InitAdaptersAttr();
    InitDefaultAdapterDevicesAttr();
    registerClient();
}

void BluetoothDbus::adapterAttrChanged(QString address, QMap<QString, QVariant> mapAttr) {
    QMap<QString, QVariant> insertAttr;
   KyDebug() << mapAttr;
    if (adapterList->contains(address))
        insertAttr = adapterList->value(address);

    if (mapAttr.contains("DefaultAdapter") && insertAttr.contains("DefaultAdapter")){
        if (mapAttr.value("DefaultAdapter").toBool() != insertAttr.value("DefaultAdapter")) {
            leaveLock(lockDev, false);
            InitDefaultAdapterDevicesAttr();
            emit defaultAdapterChanged();
        }
    }

    for (auto key : mapAttr.keys()) {
        insertAttr.remove(key);
        insertAttr.insert(key, QVariant(mapAttr.value(key)));
    }

    if (adapterList->contains(address)) {
        adapterCnt--;
        adapterList->remove(address);
    }
    adapterList->insert(address, insertAttr);
    adapterCnt++;

    if(insertAttr.contains("DefaultAdapter"))
        if (insertAttr.value("DefaultAdapter").toBool())
            defaultAdapterAttr = insertAttr;

    if(mapAttr.contains("ActiveConnection"))
    {
        emit adapterAutoConnChanged(mapAttr.value("ActiveConnection").toBool());
    }

    if (mapAttr.contains("Powered")) {
        if (insertAttr.value("DefaultAdapter").toBool()) {
            bool powered = mapAttr.value("Powered").toBool();
            emit powerChangedSignal(powered);
            if (leaveLockon)
                leaveLock(lockDev, powered);
            if (sessDbusInterface != nullptr)
                sessDbusInterface->emitPoweredSignal(powered);
        }
    }

    if (mapAttr.contains("trayShow")
            && insertAttr.contains("DefaultAdapter")
            && insertAttr.value("DefaultAdapter").toBool()) {
        KyDebug();
        emit showTrayIcon(mapAttr.value("trayShow").toBool());
    }

    if(mapAttr.contains("ClearPinCode"))
        if (mapAttr.value("ClearPinCode").toBool())
            emit pairAgentCanceledSignal(QString(""));
}

void BluetoothDbus::setAdapterAttrInside(QString address, QMap<QString, QVariant> mapAttr) {
    if (adapterList->contains(address)) {
        adapterCnt--;
        adapterList->remove(address);
    }
    adapterList->insert(address, mapAttr);
    adapterCnt++;
}

void BluetoothDbus::deviceAttrChanged(QString address, QMap<QString, QVariant> mapAttr) {
    QMap<QString, QVariant> insertAttr;
    insertAttr = devicesList->value(address);
    for (auto key : mapAttr.keys()) {
        insertAttr.remove(key);
        insertAttr.insert(key, QVariant(mapAttr.value(key)));
    }
    if (!mapAttr.contains("Battery")
            && (mapAttr.value("Type").toInt() == Type::Headset
            || mapAttr.value("Type").toInt() == Type::Headphones
            || mapAttr.value("Type").toInt() == Type::AudioVideo)) {
        mapAttr.insert("Battery", QVariant(getDevBatteryLevel(address)));
    }
    if (!mapAttr.contains("Battery"))
        mapAttr.insert("Battery", QVariant(-1));
    devicesList->remove(address);
    devicesList->insert(address, insertAttr);
    if (mapAttr.contains("Paired"))
        emit pairAgentCanceledSignal(address);
    if (mapAttr.contains("Connecting") && !mapAttr.value("Connecting").toBool())
        emit pairAgentCanceledSignal(address);
    if (insertAttr.value("Paired").toBool()) {
        pairedList->insert(address, insertAttr);

        if (sessDbusInterface != nullptr)
            sessDbusInterface->emitPairedSignal(address, true);
    }
    if (pairedList->contains(address)) {
        if (mapAttr.value("Connected").toBool()) {
            if (m_Order.contains(address))
                m_Order.removeOne(address);
            m_Order.push_front(address);
        } else if (m_Order.contains(address)) {
            m_Order.removeOne(address);
            m_Order.push_back(address);
        }
    }
}

void BluetoothDbus::setDeviceAttrInside(QString address, QMap<QString, QVariant> mapAttr) {
    if (devicesList->keys().contains(address))
        devicesList->remove(address);
    if (!mapAttr.contains("Battery")
            && (mapAttr.value("Type").toInt() == Type::Headset
            || mapAttr.value("Type").toInt() == Type::Headphones
            || mapAttr.value("Type").toInt() == Type::AudioVideo)) {
        mapAttr.insert("Battery", QVariant(getDevBatteryLevel(address)));
    }
    if (!mapAttr.contains("Battery"))
        mapAttr.insert("Battery", QVariant(-1));
    devicesList->insert(address, mapAttr);
    if (mapAttr.contains("Paired") && mapAttr.value("Paired").toBool()) {
        pairedList->insert(address, mapAttr);

        if (sessDbusInterface == nullptr)
            return;

        sessDbusInterface->emitPairedSignal(address, true);
    }
    if (pairedList->contains(address)) {
        if (mapAttr.value("Connected").toBool()) {
            if (m_Order.contains(address))
                m_Order.removeOne(address);
            m_Order.push_front(address);
        } else if (m_Order.contains(address)) {
            m_Order.removeOne(address);
            m_Order.push_back(address);
        }
    }
}

int BluetoothDbus::getDevBatteryLevel(QString address) {
    QDBusInterface iface(MEDIA_BATTERY_SERVICE,
                         MEDIA_BATTERY_PATH,
                         MEDIA_BATTERY_INTERFACE,
                         QDBusConnection::sessionBus());
    if (!iface.isValid())
        return -1;
    else {
        QDBusMessage battery_response = iface.call("getBatteryLevel", address);
        int battery_int = battery_response.arguments().takeFirst().toInt();
        if(battery_int)
            return battery_int;
        else
            return -1;
    }
}

void BluetoothDbus::getAdapterAttr(QString address) {
    QMap<QString, QVariant> mapAttr;
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("getAdapterAttr");
    dbusMsg << address << QString("");
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        mapAttr = QDBusReply<QMap<QString, QVariant>>(response);
    }
    if (mapAttr.isEmpty()) {
        INFO_PRINT("Adapter" << address << "Init Error");
        return;
    }
    setAdapterAttrInside(address, mapAttr);
}

void BluetoothDbus::InitAdaptersAttr() {
    QStringList adpsAddrList;
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("getAllAdapterAddress");
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        adpsAddrList = response.arguments().takeFirst().toStringList();
    }
    if (adpsAddrList.isEmpty()) {
        INFO_PRINT("Adapter Address Init Error");
        return;
    }
    for (int i = 0; i < adpsAddrList.size(); i++) {
        getAdapterAttr(adpsAddrList.at(i));
    }
    adapterCnt = adapterList->size();
    if(adapterList->size() == 1)
        defaultAdapterAttr = adapterList->values().at(0);
    else {
        for (auto addr : adapterList->keys()) {
            if (adapterList->value(addr).value("DefaultAdapter").toBool()) {
                defaultAdapterAttr = adapterList->value(addr);
                return;
            }
        }
    }
}

void BluetoothDbus::getDevAttr(QString address) {
    QMap<QString, QVariant> mapAttr;
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("getDevAttr");
    dbusMsg << address;
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        mapAttr = QDBusReply<QMap<QString, QVariant>>(response);
    }
    if (mapAttr.isEmpty()) {
        INFO_PRINT("Device" << address << "Init Error");
        return;
    }
    setDeviceAttrInside(address, mapAttr);
}

void BluetoothDbus::InitDefaultAdapterDevicesAttr() {
    pairedList->clear();
    QStringList devsAddrList;
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("getDefaultAdapterPairedDev");
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        devsAddrList = response.arguments().takeFirst().toStringList();
    }
    if (devsAddrList.isEmpty()) {
        INFO_PRINT("Adapter Address Init Error");
        return;
    }
    for (int i = 0; i < devsAddrList.size(); i++) {
        getDevAttr(devsAddrList.at(i));
    }
}

void BluetoothDbus::devBatteryChangedSignalSlot(QString address, int battery) {
    if (envPC == Environment::HUAWEI)
        return;
    if (!pairedList->keys().contains(address))
        return;
    KyDebug() << "devBatteryChangedSignalSLot" << address << battery;
    QMap<QString, QVariant> mapAttr = pairedList->value(address);
    if (mapAttr.value("Battery").toInt() != -1)
        return;
    mapAttr.remove("Battery");
    mapAttr.insert("Battery", QVariant(battery));
    pairedList->remove(address);
    pairedList->insert(address, mapAttr);
    emit devAttrChanged(address, pairedList->value(address));
}

void BluetoothDbus::adapterAttrChangedSLot(QString address, QMap<QString, QVariant> adapterAttr) {
    if (!adapterList->contains(address))
        return;
    adapterAttrChanged(address, adapterAttr);
}

void BluetoothDbus::adapterRemoveSignalSLot(QString address) {
   KyDebug() << address;
    adapterList->remove(address);
    --adapterCnt;
    if (adapterList->isEmpty() || adapterList->size() == 0 || adapterCnt == 0) {
        leaveLock(lockDev, false);
        emit existAdapter(false, false);
        sessDbusInterface->adapterRemoveSignal(address);
    }
}

void BluetoothDbus::adapterAddSignalSLot(QMap<QString, QVariant> adapterAttr) {
   KyDebug() ;
    QString address = adapterAttr.value("Addr").toString();
    setAdapterAttrInside(address, adapterAttr);
    emit existAdapter(true, defaultAdapterAttr.value("trayShow").toBool());
    emit adapterAddSignal(address);
    sessDbusInterface->emitAdapterAddSignal(address);
}

void BluetoothDbus::adapterChangedSLot(QString address, QMap<QString, QVariant> adapterAttr) {
   KyDebug() << address;
    if (adapterAttr.value("DefaultAdapter").toBool() && defaultAdapterAttr.value("Addr").toString() != address) {
        InitDefaultAdapterDevicesAttr();
        emit adapterChangedSignal();
        sessDbusInterface->emitDefaultAdapterChangedSignal(address);
    } else
        setAdapterAttrInside(address, adapterAttr);
}

void BluetoothDbus::deviceAddSLot(QMap<QString, QVariant> deviceAttr) {
    KyDebug() ;
    setDeviceAttrInside(deviceAttr.value("Addr").toString(), deviceAttr);
}

void BluetoothDbus::deviceAttrChangedSLot(QString address, QMap<QString, QVariant> deviceAttr) {
    if (!devicesList->contains(address))
        return;
    KyDebug() << Q_FUNC_INFO << deviceAttr << __LINE__ ;
    deviceAttrChanged(address, deviceAttr);
    if (deviceAttr.contains("Paired"))
        emit deviceAddSignal(pairedList->value(address));
    if (deviceAttr.contains("ConnectFailedId") && pairedList->contains(address))
    {
        if (pairedList->value(address).value("ShowName") != "")
            emit connectionErrorMsg(pairedList->value(address).value("ShowName").toString(),
                                    deviceAttr.value("ConnectFailedId").toInt());
        else
            emit connectionErrorMsg(pairedList->value(address).value("Name").toString(),
                                    deviceAttr.value("ConnectFailedId").toInt());
    }
    emit devAttrChanged(address, pairedList->value(address));
}

void BluetoothDbus::devRemoveSignalSLot(QString address, QMap<QString, QVariant> attr) {
    if (attr.value("Adapter").toString() != defaultAdapterAttr.value("Addr").toString())
        return;
    if (devicesList->contains(address))
        devicesList->remove(address);
    if (!pairedList->contains(address))
        return;
    KyDebug() ;
    pairedList->remove(address);
    if (lockDev == address) {
        leaveLock(address, false);
        lockDev = "";
        setLeaveLock(lockDev, false);
    }
    if (m_Order.contains(address))
        m_Order.removeOne(address);
    emit devRemoveSignal(address);

    if (sessDbusInterface == nullptr)
        return;

    sessDbusInterface->emitPairedSignal(address, false);
}

void BluetoothDbus::devErrorSignalSLot(QString a) {
    KyDebug() << "-----------------------" << a;
}

void BluetoothDbus::devPairSignalSLot(QMap<QString, QVariant> pairAttr) {
   KyDebug() ;
    if (pairAttr.value("type").toInt())
        emit displayPasskeySignal(pairAttr.value("dev").toString(),
                                  pairAttr.value("name").toString(),
                                  pairAttr.value("pincode").toString());
    else
        emit requestConfirmationSignal(pairAttr.value("dev").toString(),
                                       pairAttr.value("name").toString(),
                                       pairAttr.value("pincode").toString());
}

void BluetoothDbus::receiveFilesSlot(QMap<QString, QVariant> info) {
    KyDebug() << Q_FUNC_INFO << info << __LINE__ ;
    if (!pairedList->contains(info.value("dev").toString()))
        return;

    QString name = info.value("name").toString();
    QMap<QString, QVariant> device = pairedList->value(info.value("dev").toString());

    if (device.contains("ShowName") && device.value("ShowName").toString() != QString(""))
        name = device.value("ShowName").toString();

    emit receiveFilesSignal(info.value("dev").toString(),
                            name,
                            info.value("filename").toString(),
                            info.value("filetype").toString(),
                            info.value("filesize").toULongLong());
}

bool BluetoothDbus::sendFiles(QString address, QStringList filelist) {
   KyDebug() ;
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("sendFiles");
    dbusMsg << userName << address << filelist;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Send Succeed");
            return 0;
        default:
            DEBUG_PRINT("Send Failed");
            return -1;
        }
    }  else {
        DEBUG_PRINT("Method Call Error");
        return -1;
    }
}

void BluetoothDbus::cancelFileTransfer(QString address, int opt) {
   KyDebug() ;
    QMap<QString, QVariant> sendMsg;
    sendMsg.insert("dev", QVariant(address));
    sendMsg.insert("transportType", QVariant(opt));
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("stopFiles");
    dbusMsg << sendMsg;
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Send Succeed");
            break;
        default:
            DEBUG_PRINT("Send Failed");
        }
    }  else {
        DEBUG_PRINT("Method Call Error");
    }
}

void BluetoothDbus::openBluetoothSettings() {
   KyDebug() ;
    Config::OpenBluetoothSettings();
}

void BluetoothDbus::activeConnectionReply(QString dev, bool v) {
   KyDebug() ;
    Config::activeConnectionReply(dev, v);
}

void BluetoothDbus::pairFuncReply(QString address, bool accept) {
    KyDebug() << "pair with :" << address << " reply :" << accept;
    Config::pairFuncReply(address, accept);
}


void BluetoothDbus::replyFileReceiving(QString address, bool accept) {
   KyDebug() << QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) << QDir::homePath();
    Config::replyFileReceiving(address,
                               accept,
                               QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                               QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}

void BluetoothDbus::devConnect(const QString address) {
   KyDebug() ;
    Config::devConnect(address);
}

void BluetoothDbus::devDisconnect(const QString address) {
   KyDebug() ;
    Config::devDisconnect(address);
}

void BluetoothDbus::setDefaultAdapterPower(bool power) {
    KyDebug() << Q_FUNC_INFO <<defaultAdapterAttr.value("Powered").toBool() << power<< __LINE__ ;
    QMap<QString, QVariant> Attr;
    if (power == defaultAdapterAttr.value("Powered").toBool())
        return;
    KyDebug() << Q_FUNC_INFO <<defaultAdapterAttr.value("Powered").toBool() << power<< __LINE__ ;
    Attr.insert("Powered", QVariant(power));
    Config::setDefaultAdapterAttr(Attr);
}

QMap<QString, QMap<QString, QVariant>> BluetoothDbus::getSendableDevices() {
   KyDebug() ;
    QMap<QString, QMap<QString, QVariant>> ret;
    for (auto addr : pairedList->keys()) {
        if (pairedList->value(addr).value("FileTransportSupport").toBool()) {
            QMap<QString, QVariant> inode = pairedList->value(addr);
            ret.insert(addr, inode);
        }
    }
    return ret;
}

bool BluetoothDbus::getDevSupportFileSend(QString address) {
   KyDebug() ;
    if (!pairedList->contains(address))
        return false;
    return pairedList->value(address).value("FileTransportSupport").toBool();;
}

bool BluetoothDbus::getTrayIconShow() {
   KyDebug();
    if (adapterList->isEmpty())
        return false;
    return defaultAdapterAttr.value("trayShow").toBool();
}

bool BluetoothDbus::getExistAdapter() {
   KyDebug();
    return !adapterList->isEmpty();
}

bool BluetoothDbus::isPowered() {
    KyDebug() << Q_FUNC_INFO << defaultAdapterAttr.value("Powered").toBool() << __LINE__ ;
    return defaultAdapterAttr.value("Powered").toBool();
}

QMap<QString, QMap<QString, QVariant>> BluetoothDbus::getPairedDevices() {
   KyDebug() ;
    return *pairedList;
}

QStringList BluetoothDbus::getOrderList() {
    KyDebug() ;
    return m_Order;
}

void BluetoothDbus::pingTimeSignalSLot(QByteArray time) {
    if (!leaveLockon) {
        KyDebug() << "leave lock off";
        return;
    }
    QString s_info = QString(time);
    KyDebug() << time;
    if (s_info.contains("Connection reset by peer")) {
        reping = true;
        leaveLock(lockDev, leaveLockon);
        return;
    }
    if (s_info.contains("loss connect") || (s_info.contains("Host is down") && reping)) {
        reping = false;
        QTimer::singleShot(60000,this, [=](){
            KyDebug() << "++++++Command to lock Screen";
            system("lingmo-screensaver-command -l");
        });
        return;
    }
    if (s_info.contains("Host is down") && !reping) {
        return;
    }
    QString s_time = s_info.right(9).left(6);
    int i_time = s_time.toDouble();
    KyDebug() << s_time << i_time;
    if (i_time == 0) {
        bool removedOne = false;
        if (s_time.at(0) == ' ') {
            s_time.remove(0,1);
            removedOne = true;
        }
        if (s_time.at(1) == ' ') {
            s_time.remove(0,2);
            removedOne = true;
        }
        if (removedOne) {
            i_time = s_time.toDouble();
            getPingTimeList(i_time);
        }
    } else {
        getPingTimeList(i_time);
    }
}

void BluetoothDbus::getPingTimeList(int i_time) {
    if (!leaveLockon)
        return;
    if (leaveLockCount[ leaveLockCount[PINT_TIME_COUNT] ] >= LOCK_PING_TIME)
        leaveLockCount[TIME_OUT_COUNT] -= 1;

    leaveLockCount[ leaveLockCount[PINT_TIME_COUNT] ] = i_time;
    leaveLockCount[PINT_TIME_COUNT] += 1;

    if (leaveLockCount[PINT_TIME_COUNT] == LENGTH_OF_TIME_VECTOR)
        leaveLockCount[PINT_TIME_COUNT] = 0;

    if (i_time >= LOCK_PING_TIME)
        leaveLockCount[TIME_OUT_COUNT] += 1;

    if (leaveLockCount[TIME_OUT_COUNT] >= MAX_TIME_OUT  && leaveLockon) {
        KyDebug() << "LockScren" ;
        if (!timerActive) {
            timerActive = true;
            QTimer::singleShot(55000,this, [=](){
                timerActive = false;
                if (reping) {
                    leaveLock(lockDev, leaveLockon);
                }
                if (leaveLockCount[TIME_OUT_COUNT] >= MAX_TIME_OUT && leaveLockon) {
                    KyDebug() << "++++++Command to lock Screen";
                    system("lingmo-screensaver-command -l");
                }
            });
        }
    }
    /* //DEBUG
    for (int i = 0; i < LENGTH_OF_ALL_VECTOR; i++) {
        printf("%d ", leaveLockCount[i]);
    }
    printf("\n");
    */
}

void BluetoothDbus::setLeaveLock(QString address, bool on) {
    if(leaveLockon == on) {
        return;
    }
    if (address == "") {
        leaveLockon = false;
    }
    QGSettings(GSETTING_SCHEMA_LINGMOBLUETOOH).set("leave-lock", QVariant(on));
    QGSettings(GSETTING_SCHEMA_LINGMOBLUETOOH).set("leavelock-dev", QVariant(address));
    leaveLock(address, on);
}

void BluetoothDbus::leaveLock(QString address, bool on) {
    if(address.isEmpty() && leaveLockon == on) {
        return;
    }
    lockDev = address;
    if (address == "") {
        leaveLockon = false;
    }
    KyDebug() << address << on ;
    for (int i = 0; i < LENGTH_OF_ALL_VECTOR; i++) {
        leaveLockCount[i] = 0;
    }
     QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("setLeaveLock");
     dbusMsg << QDBusConnection::systemBus().baseService() << address<< on;
     QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);
     leaveLockon = on;
     sessDbusInterface->setLeaveLockOn(leaveLockon);
     sessDbusInterface->setLeaveLockDev(lockDev);
}

void BluetoothDbus::unlockSlot() {
    if (!leaveLockon)
        return;
    QTimer::singleShot(3000, this, [=](){
        leaveLock(lockDev, true);
    });
}

void BluetoothDbus::lockSlot() {
    QTimer::singleShot(3000, this, [=](){
        leaveLock(lockDev, false);
    });
}

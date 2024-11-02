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

#include "sessiondbusinterface.h"

SessionDbusInterface::SessionDbusInterface(QMap<QString, QVariant> *_adapterAttr, QMap<QString, QMap<QString, QVariant>> *_adapterList,
                                           QMap<QString, QMap<QString, QVariant>> *_deviceList, QObject *parent) : QObject(parent)
{
    adapterAttr = _adapterAttr;
    deviceList = _deviceList;
    adapterList = _adapterList;
    KyInfo() ;
    if (InterfaceAlreadyExists()) {
        QDBusConnection sessionDbus = QDBusConnection::sessionBus();
        if (!sessionDbus.registerService("com.lingmo.bluetooth")) {
            qCritical() << "QDbus register service failed reason:" << sessionDbus.lastError();
        }

        if (!sessionDbus.registerObject("/com/lingmo/bluetooth", "com.lingmo.bluetooth", this, QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals)){
            qCritical() << "QDbus register object failed reason:" << sessionDbus.lastError();
        }
        KyInfo() ;
    }
    initMprisMediaDbusConnect();
}

void SessionDbusInterface::ConnectSystemDbusSignal() {

}

bool SessionDbusInterface::InterfaceAlreadyExists()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected())
        return false;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", "com.lingmo.bluetooth");
    KyInfo()  << reply.value();
    return reply.value() == "";
}

bool SessionDbusInterface::registerClient(QString dbusid, QString username, int type, quint64 pid) {
    QMap<QString, QVariant> rcParam;
    rcParam.insert("dbusid", dbusid);
    rcParam.insert("username", username);
    rcParam.insert("type", type);
    rcParam.insert("pid", pid);
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("registerClient");
    dbusMsg << rcParam;
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        return response.arguments().takeFirst().toBool();
    } else
        return false;
}

bool SessionDbusInterface::unregisterClient(QString dbusid) {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("unregisterClient");
    dbusMsg << dbusid;
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        return response.arguments().takeFirst().toBool();
    } else
        return false;
}

bool SessionDbusInterface::getBluetoothBlock() {
    return adapterAttr->value("Block").toBool();
}

QString  SessionDbusInterface::getDefaultAdapterAddress(){
    return adapterAttr->value("Addr").toString();
}

bool SessionDbusInterface::getDefaultAdapterPower() {
    return adapterAttr->value("Powered").toBool();
}

bool SessionDbusInterface::getDefaultAdapterScanStatus() {
    return adapterAttr->value("Discovering").toBool();
}

bool SessionDbusInterface::getDefaultAdapterDiscoverable() {
    return adapterAttr->value("Discoverable").toBool();
}

QString SessionDbusInterface::getAdapterNameByAddr(QString address) {
    if (!adapterList->keys().contains(address))
        return "";
    return adapterList->value(address).value("Name").toString();
}

QStringList SessionDbusInterface::getDefaultAdapterPairedDevAddress() {
    QStringList pairedList;
    for (auto i : deviceList->keys()) {
        if (deviceList->value(i).value("Paired").toBool())
            pairedList.append(i);
    }
    return pairedList;
}

QStringList SessionDbusInterface::getPairedPhoneAddr() {
    QStringList pairedList;
    if (adapterList->keys().isEmpty()) {
        return pairedList;
    }
    for (auto i : deviceList->keys()) {
        if (deviceList->value(i).value("Type").toInt() != Type::Phone)
            continue;
        KyDebug() << "phone" << i;
        if (!deviceList->value(i).value("Paired").toBool())
            continue;
        pairedList.append(i);
    }
    return pairedList;
}

QStringList SessionDbusInterface::getDefaultAdapterTrustedDevAddress() {
    QStringList trustedList;
    for (auto i : deviceList->keys()) {
        if (deviceList->value(i).value("Trusted").toBool())
            trustedList.append(i);
    }
    return trustedList;
}

QStringList SessionDbusInterface::getDefaultAdapterCacheDevAddress() {
    QStringList cacheList;
    for (auto i : deviceList->keys()) {
        cacheList.append(i);
    }
    return cacheList;
}

QStringList SessionDbusInterface::getAdapterDevAddressList() {
    QStringList _adapterList;
    for (auto i : adapterList->keys()) {
        _adapterList.append(i);
    }
    return _adapterList;
}

void SessionDbusInterface::setDefaultAdapterPower(bool powered) {
    QMap<QString, QVariant> Attr;
    if (powered == adapterAttr->value("Powered").toBool())
        return;
    Attr.insert("Powered", QVariant(powered));
    Config::setDefaultAdapterAttr(Attr);
}

void SessionDbusInterface::setDefaultAdapterScanOn(bool dicovering) {
    QMap<QString, QVariant> Attr;
    if (dicovering == adapterAttr->value("Discovering").toBool())
        return;
    Attr.insert("Discovering", QVariant(dicovering));
    Config::setDefaultAdapterAttr(Attr);
}

void SessionDbusInterface::setDefaultAdapter(QString address) {
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("setDefaultAdapter");
    dbusMsg << address;
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        switch(response.arguments().takeFirst().toInt()) {
        case 0 :
            DEBUG_PRINT("set default adapter Succeed");
            break;
        case -1 :
            DEBUG_PRINT("set default adapter Error");
            break;
        case -2 :
            DEBUG_PRINT("default adapter doesn't Exist");
            break;
        }
    } else
        DEBUG_PRINT("call 'setDefaultAdapter' Failed");
}

void SessionDbusInterface::setDefaultAdapterDiscoverable(bool discoverable) {
    QMap<QString, QVariant> Attr;
    if (discoverable == adapterAttr->value("Discoverable").toBool())
        return;
    Attr.insert("Discoverable", QVariant(discoverable));
    Config::setDefaultAdapterAttr(Attr);
}

void SessionDbusInterface::setBluetoothBlock(bool) {
    return;
}

void SessionDbusInterface::setDefaultAdapterName(QString name) {
    QMap<QString, QVariant> Attr;
    if (name == adapterAttr->value("Name").toString())
        return;
    Attr.insert("Name", QVariant(name));
    Config::setDefaultAdapterAttr(Attr);
}

void SessionDbusInterface::clearNonViableDevice(QStringList) {
    return;
}

void SessionDbusInterface::devPair(const QString address) {
    Config::devConnect(address);
}

void SessionDbusInterface::devConnect(const QString address) {
    Config::devConnect(address);
}

void SessionDbusInterface::devDisconnect(const QString address) {
    Config::devDisconnect(address);
}

void SessionDbusInterface::devRemove(const QString address) {
    Config::devRemove(address, adapterAttr->value("Addr").toString());
}

void SessionDbusInterface::devTrust(const QString address,const bool trusted) {
    if (!deviceList->keys().contains(address)) {
        return;
    }
    for (auto i = deviceList->begin(); i != deviceList->end(); i++) {
        if (i.key() == address) {
            i->remove("Trusted");
            i->insert("Trusted", QVariant(trusted));
        }
    }
    QMap<QString, QVariant> devAttr;
    devAttr.insert("Name", QVariant(deviceList->value(address).value("Name").toString()));
    devAttr.insert("Trusted", QVariant(trusted));
    QDBusMessage dbusMsg = CREATE_SYSTEM_METHOD_CALL("setDevAttr");
    dbusMsg << devAttr;
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        int res = response.arguments().takeFirst().toInt();
        switch (res) {
        case 0:
            DEBUG_PRINT("Remove Succeed");
            break;
        case -1:
            DEBUG_PRINT("Address Error");
            break;
        case -2:
            DEBUG_PRINT("Devices doesn't Exist");
            break;
        }
    } else
        DEBUG_PRINT("Method Call Error");
}

bool SessionDbusInterface::getDevPairStatus(const QString address) {
    if (!deviceList->keys().contains(address)) {
        return false;
    }
    return deviceList->value(address).value("Paired").toBool();
}

bool SessionDbusInterface::getDevConnectStatus(const QString address) {
    if (!deviceList->keys().contains(address)) {
        return false;
    }
    return deviceList->value(address).value("Connected").toBool();
}

QString SessionDbusInterface::getDevName(const QString address) {
    if (adapterList->isEmpty())
        return "";
    if (!deviceList->keys().contains(address)) {
        return "";
    }
    if (deviceList->value(address).keys().contains("ShowName")) {
        if (deviceList->value(address).value("ShowName") != "") {
            return deviceList->value(address).value("ShowName").toString();}
    }
    return deviceList->value(address).value("Name").toString();
}

QString SessionDbusInterface::getDevType(const QString address) {
    if (!deviceList->keys().contains(address)) {
        return "";
    }
    switch(deviceList->value(address).value("Type").toInt()) {
    case Type::Phone:
        return QString("phone");
    case Type::Modem:
        return QString("modem");
    case Type::Computer:
        return QString("computer");
    case Type::Network:
        return QString("network");
    case Type::Headset:
        return QString("headset");
    case Type::Headphones:
        return QString("headphones");
    case Type::AudioVideo:
        return QString("audiovideo");
    case Type::Keyboard:
        return QString("keyboard");
    case Type::Mouse:
        return QString("mouse");
    case Type::Joypad:
        return QString("joypad");
    case Type::Tablet:
        return QString("tablet");
    case Type::Peripheral:
        return QString("peripheral");
    case Type::Camera:
        return QString("camera");
    case Type::Printer:
        return QString("printer");
    case Type::Imaging:
        return QString("imaging");
    case Type::Wearable:
        return QString("wearable");
    case Type::Toy:
        return QString("toy");
    case Type::Health:
        return QString("health");
    case Type::Uncategorized:
    default:
        return QString("uncategorized");
    }
}

void SessionDbusInterface::clearAllUnPairedDevicelist() {
    return;
}

void SessionDbusInterface::setSendTransferDeviceMesg(QString address) {
    emit sendTransferDeviceMesg(address);
}

void SessionDbusInterface::setSendTransferFileMesg(QStringList files) {
    emit sendTransferFilesMesg(files);
}

void SessionDbusInterface::setclearOldSession() {
    emit clearOldSession();
}

void SessionDbusInterface::continueSendFiles(QString filename) {
    emit continueSendFilesSignal(filename);
}

void SessionDbusInterface::cancelFileReceiving() {
    emit cancelFileReceivingSignal();
}

int SessionDbusInterface::getDevBattery(const QString address) {
    return deviceList->value(address).value("Battery").toInt();
}

bool SessionDbusInterface::getDevSupportFileSend(const QString address) {
    if (!deviceList->keys().contains(address))
        return false;
    return deviceList->value(address).value("FileTransportSupport").toBool();
}

int SessionDbusInterface::getDevRssi(const QString address) {
    return deviceList->value(address).value("Rssi").toInt();
}

void SessionDbusInterface::sendFiles(QString addr,QString filename) {
    if (filename.isEmpty() || filename.isNull())
        return;

    emit sendFile(addr,filename);
}

void  SessionDbusInterface::exit() {
    return;
}

void SessionDbusInterface::sendNotifyMessage(QString message) {
    Config::SendNotifyMessage(QString(tr("Bluetooth Message")), message, QString("mute"));
}

void SessionDbusInterface::openBluetoothSettings() {
    Config::OpenBluetoothSettings();
}

void SessionDbusInterface::connectSignal() {
    return;
}

void SessionDbusInterface::showTrayWidgetUI() {
    emit showTrayWidgetUISignal();
}

bool SessionDbusInterface::getTransferDevAndStatus(QString dev) {
    return true;
}

void SessionDbusInterface::sendContinueRecieveFilesSignal() {
    emit continueRecieveFilesSignal();
}

void SessionDbusInterface::sendReplyRequestConfirmation(bool v) {
    emit replyRequestConfirmation(v);
}

void SessionDbusInterface::sendReplyFileReceivingSignal(bool v) {
    emit replyFileReceivingSignal(v);
}

void SessionDbusInterface::sendCloseSession() {
    emit closeSessionSignal();
}

void SessionDbusInterface::activeConnectionReply(QString dev, bool v) {
    Config::activeConnectionReply(dev, v);
}

//多媒体键值处理
void SessionDbusInterface::initMprisMediaDbusConnect()
{
    CONNECT_SYSTEM_DBUS_SIGNAL("VolumeDown",VolumeDownSlot());
    CONNECT_SYSTEM_DBUS_SIGNAL("VolumeUp",VolumeUpSlot());
    CONNECT_SYSTEM_DBUS_SIGNAL("Next",NextSlot());
    CONNECT_SYSTEM_DBUS_SIGNAL("PlayPause",PlayPauseSlot());
    CONNECT_SYSTEM_DBUS_SIGNAL("Previous",PreviousSlot());
    CONNECT_SYSTEM_DBUS_SIGNAL("Stop",StopSlot());
    CONNECT_SYSTEM_DBUS_SIGNAL("Play",PlaySlot());
    CONNECT_SYSTEM_DBUS_SIGNAL("Pause",PauseSlot());
}

void SessionDbusInterface::VolumeDownSlot(){
    KyInfo();
    controlMprisPlayerDbus("VolumeDown");
}

void SessionDbusInterface::VolumeUpSlot(){
    KyInfo();
    controlMprisPlayerDbus("VolumeUp");
}

void SessionDbusInterface::NextSlot(){
    KyInfo();
    controlMprisPlayerDbus("Next");
}

void SessionDbusInterface::PlayPauseSlot(){
    KyInfo();
    controlMprisPlayerDbus("PlayPause");

}

void SessionDbusInterface::PreviousSlot(){
    KyInfo();
    controlMprisPlayerDbus("Previous");

}

void SessionDbusInterface::StopSlot(){
    KyInfo();
    controlMprisPlayerDbus("Stop");
}

void SessionDbusInterface::PlaySlot(){
    KyInfo();
    controlMprisPlayerDbus("Play");
}

void SessionDbusInterface::PauseSlot(){
    KyInfo();
    controlMprisPlayerDbus("Pause");
}

//获取session bus总线上注册的dbus名称
QStringList SessionDbusInterface::sessionDbusListNames()
{
    KyDebug ();

    QDBusInterface iface(SESSION_DBUS_FREEDESKTOP_SERVICE,
                         SESSION_DBUS_FREEDESKTOP_PATH,
                         SESSION_DBUS_FREEDESKTOP_INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间, ms
    iface.setTimeout(300);
    QDBusReply<QStringList> reply=iface.call("ListNames");
    return reply;
}
//
void SessionDbusInterface::controlMprisPlayerDbus(const QString funcName)
{
    KyDebug() << funcName ;
    QStringList sessionDbusNamesList = sessionDbusListNames();
    QStringList mprisDbusNameList = sessionDbusNamesList.filter(SESSION_DBUS_NAME_STR_FIELD,Qt::CaseInsensitive);
    if (mprisDbusNameList.isEmpty())
    {
        KyInfo() << "not mpris dbus" ;
        return;
    }
    KyInfo() << "funcName:" <<funcName << "dbusNameList:" << mprisDbusNameList ;
    //下发控制操作
    for (auto &dbusName:mprisDbusNameList) {
        //去掉对lingmo-music控制
        if (dbusName == "org.mpris.MediaPlayer2.LingmoMusic")
        {
            KyInfo() << "===========================" << dbusName;
            continue;
        }
        QDBusInterface iface(dbusName,
                             SESSION_DBUS_PLAYER_PATH,
                             SESSION_DBUS_PLAYER_INTERFACE,
                             QDBusConnection::sessionBus());
        iface.setTimeout(300);
        iface.call(funcName);
    }
}

void SessionDbusInterface::setLeaveLock(QString address, bool on) {
    emit setLeaveLockSignal(address, on);
}

void SessionDbusInterface::emitPairedSignal(QString address, bool paired) {
    emit devPairSignal(address, paired);
}

void SessionDbusInterface::emitPoweredSignal(bool powered) {
    emit defaultAdapterPowerChanged(powered);
}

void SessionDbusInterface::setLeaveLockOn(bool leaveLock) {
    isLeaveLock = leaveLock;
}

void SessionDbusInterface::setLeaveLockDev(QString lockDev) {
    leaveLockDev = lockDev;
}

bool SessionDbusInterface::getLeaveLock() {
    return isLeaveLock;
}

QString SessionDbusInterface::getLeaveLockDev() {
    return leaveLockDev;
}

void SessionDbusInterface::emitAdapterRemoveSignal(QString address) {
    emit adapterRemoveSignal(address);
}

void SessionDbusInterface::emitAdapterAddSignal(QString address) {
    emit adapterAddSignal(address);
}

void SessionDbusInterface::emitDefaultAdapterChangedSignal(QString address) {
    emit defaultAdapterChanged(address);
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothmanager.h"
#include "dbluetoothadapter.h"
#include "dbluetoothdevice.h"
#include "dbluetoothobexmanager.h"
#include "dbluetoothobexsession.h"
#include "dbluetoothobextransfer.h"
#include <QDebug>

DBLUETOOTH_USE_NAMESPACE

int main()
{
    // bluetooth
    DManager manager;
    if (!manager.available()) {
        qWarning() << "bluez unsupported.";
        return -1;
    }
    const auto &reply1 = manager.adapters();
    if (!reply1) {
        qWarning() << reply1.error();
        return -1;
    }
    const auto &adapterList = reply1.value();
    if (adapterList.empty()) {
        qWarning() << "bluetooth adapter doesn't exists";
        return -1;
    }
    const auto &reply2 = manager.adapterFromId(adapterList.first());
    if (!reply2) {
        qWarning() << reply2.error();
        return -1;
    }
    const auto &adapter = reply2.value();
    qDebug() << adapter->address() << adapter->addressType() << adapter->discoverable();
    const auto &reply3 = adapter->devices();
    if (!reply3) {
        qWarning() << reply3.error();
        return -1;
    }
    const auto &deviceList = reply3.value();
    const auto &reply4 = adapter->deviceFromAddress(deviceList.first());
    if (!reply4) {
        qWarning() << reply4.error();
        return -1;
    }
    const auto &device = reply4.value();
    device->setTrusted(true);
    const auto &reply5 = device->pair();
    if (!reply5) {
        qWarning() << reply5.error();
        return -1;
    }
    if (device->paired()) {
        const auto &tmp = device->connectDevice();
        if (!tmp) {
            qWarning() << tmp.error();
            return -1;
        }
    } else {
        qWarning() << "bluetooth pair error.";
        return -1;
    }

    // obex

    DObexManager obex;
    const auto &reply6 = obex.createSession(
        device->address(),
        {{"Target", QVariant::fromValue<QString>("opp")}, {"Source", QVariant::fromValue<QString>(adapter->address())}});
    if (!reply6) {
        qWarning() << reply6.error();
        return -1;
    }
    const auto &reply7 = obex.sessionFromInfo(reply6.value());
    if (!reply7) {
        qWarning() << reply7.error();
        return -1;
    }
    const auto &session = reply7.value();
    QFile tmpfile{"/tmp/1.txt"};
    tmpfile.open(QFile::OpenModeFlag::WriteOnly);
    tmpfile.write(QByteArray{"test for dtkbluetooth."});
    const auto &reply8 = session->sendFile(QFileInfo{tmpfile});
    if (!reply8) {
        qWarning() << reply8.error();
        return -1;
    }
    const auto &reply9 = session->transferFromId(reply8.value());
    if (!reply9) {
        qWarning() << reply9.error();
        return -1;
    }
    const auto &transfer = reply9.value();
    qDebug() << transfer->transferred();
    return 0;
}

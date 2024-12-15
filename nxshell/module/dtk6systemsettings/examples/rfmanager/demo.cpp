// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "demo.h"

#include <QDebug>

RfDemo::RfDemo()
{
    m_rfMgr = new DRfmanager;

    connect(m_rfMgr, &DRfmanager::bluetoothBlockedChanged, [](bool bluetoothBlocked) {
        qInfo() << "bluetoothBlockedChanged" << bluetoothBlocked;
    });
    connect(m_rfMgr, &DRfmanager::wifiBlockedChanged, [](bool wifiBlocked) {
        qInfo() << "wifiBlockedChanged" << wifiBlocked;
    });
    connect(m_rfMgr, &DRfmanager::allBlockedChanged, [](bool blockAll) {
        qInfo() << "allBlockedChanged" << blockAll;
    });
    connect(m_rfMgr, &DRfmanager::blockedChanged, [&](quint32 idx) {
        qInfo() << "blockedChanged:" << idx;
    });
}

void RfDemo::run()
{
    bool isWifiBlocked = m_rfMgr->isWifiBlocked();
    bool isBluetoothBlocked = m_rfMgr->isBluetoothBlocked();
    bool isAllBlocked = m_rfMgr->isAllBlocked();
    qInfo() << "isWifiBlocked:" << isWifiBlocked;
    qInfo() << "isBluetoothBlocked:" << isBluetoothBlocked;
    qInfo() << "isAllBlocked:" << isAllBlocked;

    m_rfMgr->blockWifi(!isWifiBlocked);
    qInfo() << "isWifiBlocked:" << m_rfMgr->isWifiBlocked();

    m_rfMgr->blockBluetooth(!isBluetoothBlocked);
    qInfo() << "isBluetoothBlocked:" << m_rfMgr->isBluetoothBlocked();

    m_rfMgr->blockAll(!isAllBlocked);
    qInfo() << "isAllBlocked:" << m_rfMgr->isAllBlocked();

    connect(m_rfMgr, &DRfmanager::countChanged, [&](int) {
        qInfo() << m_rfMgr->deviceList();
    });
}

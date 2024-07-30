/*
 * SPDX-FileCopyrightText: 2011 Rafael Fernández López <ereslibre@kde.org>
 * SPDX-FileCopyrightText: 2011 UFO Coders <info@ufocoders.com>
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "adaptersreceiver.h"

#include <QCoreApplication>
#include <QDebug>

#include "adapter.h"
#include "initmanagerjob.h"

using namespace BluezQt;

AdaptersReceiver::AdaptersReceiver(Manager *manager, QObject *parent)
    : QObject(parent)
    , m_manager(manager)
{
    connect(manager, &Manager::adapterAdded, this, &AdaptersReceiver::adapterAdded);
    connect(manager, &Manager::adapterRemoved, this, &AdaptersReceiver::adapterRemoved);
    connect(manager, &Manager::usableAdapterChanged, this, &AdaptersReceiver::usableAdapterChanged);
    connect(manager, &Manager::allAdaptersRemoved, this, &AdaptersReceiver::allAdaptersRemoved);
    connect(manager, &Manager::bluetoothOperationalChanged, this, &AdaptersReceiver::bluetoothOperationalChanged);
    connect(manager, &Manager::bluetoothBlockedChanged, this, &AdaptersReceiver::bluetoothBlockedChanged);
}

void AdaptersReceiver::adapterAdded(const BluezQt::AdapterPtr &adapter)
{
    qDebug() << "Adapter added: " << adapter->name();
    printStatus();
}

void AdaptersReceiver::adapterRemoved(const BluezQt::AdapterPtr &adapter)
{
    qDebug() << "Adapter removed: " << adapter->name();
    printStatus();
}

void AdaptersReceiver::usableAdapterChanged(const BluezQt::AdapterPtr &adapter)
{
    qDebug() << "Usable adapter changed: " << adapter;
    printStatus();
}

void AdaptersReceiver::allAdaptersRemoved()
{
    qDebug() << "All adapters have been removed";
    printStatus();
}

void AdaptersReceiver::bluetoothOperationalChanged(bool operational)
{
    qDebug() << "Bluetooth operational changed: " << operational;
    printStatus();
}

void AdaptersReceiver::bluetoothBlockedChanged(bool blocked)
{
    qDebug() << "Bluetooth blocked changed: " << blocked;
    printStatus();
}

void AdaptersReceiver::printStatus()
{
    qDebug() << "\tBluetooth Blocked: " << m_manager->isBluetoothBlocked();
    qDebug() << "\tBluetooth Operational: " << m_manager->isBluetoothOperational();
    qDebug() << "\tUsable Adapter: " << m_manager->usableAdapter();
    qDebug() << "";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    qDebug() << "Looping until stopped";

    Manager *manager = new Manager();
    InitManagerJob *initJob = manager->init();
    initJob->exec();
    if (initJob->error()) {
        qWarning() << "Error initializing manager:" << initJob->errorText();
        return 1;
    }

    new AdaptersReceiver(manager);
    return app.exec();
}

#include "moc_adaptersreceiver.cpp"

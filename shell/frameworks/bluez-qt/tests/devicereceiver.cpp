/*
 * SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>
 * SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "devicereceiver.h"

#include <QCoreApplication>
#include <QDebug>

#include "adapter.h"
#include "device.h"
#include "initmanagerjob.h"
#include "pendingcall.h"

using namespace BluezQt;

Manager *g_manager = nullptr;

DeviceReceiver::DeviceReceiver(QObject *parent)
    : QObject(parent)
{
}

void DeviceReceiver::scanDevices()
{
    AdapterPtr usableAdapter = g_manager->usableAdapter();
    if (!usableAdapter && g_manager->adapters().isEmpty()) {
        qDebug() << "!!! No bluetooth adapters were found. Waiting for bluetooth adapters. Ctrl + C to cancel...";
        connect(g_manager, &Manager::adapterAdded, this, &DeviceReceiver::adapterAdded);
        return;
    } else if (!usableAdapter) {
        usableAdapter = g_manager->adapters().at(0);
        PendingCall *powerOnCall = usableAdapter->setPowered(true);
        powerOnCall->waitForFinished();
    }

    qDebug() << "*** Will scan devices until stopped...";
    qDebug();

    connect(usableAdapter.data(), &Adapter::deviceAdded, this, &DeviceReceiver::deviceFound);
    connect(usableAdapter.data(), &Adapter::deviceChanged, this, &DeviceReceiver::devicePropertyChanged);

    usableAdapter->startDiscovery();
}

void DeviceReceiver::deviceFound(const BluezQt::DevicePtr &device)
{
    qDebug() << "*** Remote device found:" << device->name() << "(" << device->address() << ")";
    qDebug();
}

void DeviceReceiver::devicePropertyChanged(const BluezQt::DevicePtr &device)
{
    qDebug() << "*** Device with address" << device->address() << "changed some property";
    qDebug() << "\tAddress:\t" << device->address();
    qDebug() << "\tClass:\t\t" << device->deviceClass();
    qDebug() << "\tIcon:\t\t" << device->icon();
    qDebug() << "\tLegacy Pairing:\t" << (device->hasLegacyPairing() ? "yes" : "no");
    qDebug() << "\tName:\t\t" << device->name();
    qDebug() << "\tPaired:\t\t" << (device->isPaired() ? "yes" : "no");
    qDebug() << "\tTrusted:\t" << (device->isTrusted() ? "yes" : "no");
    qDebug() << "\tServices:\n" << device->uuids();
    qDebug();
}

void DeviceReceiver::adapterAdded(const BluezQt::AdapterPtr &adapter)
{
    Q_UNUSED(adapter)

    qDebug() << "*** An adapter has been connected.";
    qDebug();
    scanDevices();
}

static void stopDiscovering()
{
    if (g_manager) {
        for (const AdapterPtr &adapter : g_manager->adapters()) {
            adapter->stopDiscovery();
        }
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    qAddPostRoutine(stopDiscovering);

    g_manager = new Manager();
    InitManagerJob *initJob = g_manager->init();
    initJob->exec();

    if (initJob->error()) {
        qWarning() << "Error initializing manager:" << initJob->errorText();
        return 1;
    }

    DeviceReceiver *deviceReceiver = new DeviceReceiver();
    deviceReceiver->scanDevices();

    return app.exec();
}

#include "moc_devicereceiver.cpp"

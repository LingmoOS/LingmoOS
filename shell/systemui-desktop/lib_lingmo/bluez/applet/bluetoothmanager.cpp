/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bluetoothmanager.h"
#include <QDebug>

#include <BluezQt/InitManagerJob>
#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/MediaPlayer>
#include <BluezQt/PendingCall>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
    , m_agent(new BluetoothAgent(this))
{
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &BluezQt::InitManagerJob::result, this, &BluetoothManager::onInitJobResult);

    connect(m_agent, &BluetoothAgent::confirmationRequested, this, &BluetoothManager::confirmationRequested);

    connect(m_manager, &BluezQt::Manager::bluetoothBlockedChanged, this, [=] (bool blocked) {
        if (!blocked) {
            BluezQt::AdapterPtr adaptor = m_manager->adapters().first();
            if (adaptor) {
                if (!adaptor->isDiscoverable()) {
                    adaptor->setDiscoverable(true);
                }
            }
        }
    });
}

BluetoothManager::~BluetoothManager()
{
    m_manager->unregisterAgent(m_agent);

    delete m_agent;
    delete m_manager;
}

void BluetoothManager::setName(const QString &name)
{
    BluezQt::AdapterPtr adaptor = m_manager->usableAdapter();
    adaptor->setName(name);
}

void BluetoothManager::connectToDevice(const QString address)
{
    BluezQt::AdapterPtr adaptor = m_manager->usableAdapter();
    BluezQt::DevicePtr device = adaptor->deviceForAddress(address);
    qDebug() << "hello: " << address << device->name();
    m_device = device;
    device->setTrusted(true);
    BluezQt::PendingCall *call = m_device->connectToDevice();
    connect(call, &BluezQt::PendingCall::finished, this, &BluetoothManager::connectFinished);
//    connect(m_device.data(), &BluezQt::Device::connectedChanged, this, &Bluetooth::connectedStateChanged);
}

void BluetoothManager::requestParingConnection(const QString address)
{
    BluezQt::AdapterPtr adaptor = m_manager->usableAdapter();
    BluezQt::DevicePtr device = adaptor->deviceForAddress(address);
    m_device = device;
//    m_address =  address;
    BluezQt::PendingCall *pairCall = m_device->pair();
    connect(pairCall, &BluezQt::PendingCall::finished, this, &BluetoothManager::pairingFinished);
}

void BluetoothManager::confirmMatchButton(const bool match)
{
    if (match){
        m_req.accept();
    } else{
        m_req.reject();
    }
}

void BluetoothManager::deviceDisconnect(const QString address)
{
    stopMediaPlayer(address);
    BluezQt::AdapterPtr adaptor = m_manager->usableAdapter();
    BluezQt::DevicePtr device = adaptor->deviceForAddress(address);
    BluezQt::PendingCall *pairCall = device->disconnectFromDevice();
    //    connect(pairCall, &BluezQt::PendingCall::finished, this, &Bluetooth::disconnectFromDeviceFinished);
}

void BluetoothManager::deviceRemoved(const QString address)
{
    stopMediaPlayer(address);
    BluezQt::AdapterPtr adaptor = m_manager->usableAdapter();
    BluezQt::DevicePtr device = adaptor->deviceForAddress(address);
    BluezQt::PendingCall *removeCall = adaptor->removeDevice(device);
    // connect(removeCall, &BluezQt::PendingCall::finished, this, &BluetoothManager::removeDeviceFinished);
}

void BluetoothManager::stopMediaPlayer(const QString address)
{
    BluezQt::AdapterPtr adaptor = m_manager->usableAdapter();
    BluezQt::DevicePtr device = adaptor->deviceForAddress(address);
    BluezQt::MediaPlayerPtr mediaPlayer = device->mediaPlayer();

    if (mediaPlayer){
        mediaPlayer->stop();
    }
}

void BluetoothManager::onInitJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qDebug() << "Init Bluetooth error";
        return;
    }

    // Make sure to register agent when bluetoothd starts
    operationalChanged(m_manager->isOperational());
    connect(m_manager, &BluezQt::Manager::operationalChanged, this, &BluetoothManager::operationalChanged);

    m_adapter = m_manager->usableAdapter();
    if (m_adapter) {
        setName("LingmoOS");

        if (!m_adapter->isDiscoverable())
            m_adapter->startDiscovery();
    }
}

void BluetoothManager::operationalChanged(bool operational)
{
    if (operational) {
        m_manager->registerAgent(m_agent);
    } else {
        // Attempt to start bluetoothd
        BluezQt::Manager::startService();
    }
}

void BluetoothManager::confirmationRequested(const QString &passkey, const BluezQt::Request<> &req)
{
    m_req = req;
    Q_EMIT showPairDialog(m_device->name(), passkey);
}

void BluetoothManager::pairingFinished(BluezQt::PendingCall *call)
{
    qDebug() << call->error() << call->errorText();
    if (!call->error()) {
        // Success
    } else {
        if (call->error() == 98) {
            // showPairDialog
        } else {
            emit pairFailed(m_device->name(), m_device->type());
        }
    }
}

void BluetoothManager::connectFinished(BluezQt::PendingCall *call)
{
    if (call->error()) {
        emit connectFailed(m_device->name(), m_device->type());
    }
}

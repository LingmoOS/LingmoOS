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

#include "bluetoothagent.h"
#include "adapter.h"
#include "device.h"


BluetoothAgent::BluetoothAgent(QObject *parent)
    : Agent(parent)
{
    KyInfo();

}

QDBusObjectPath BluetoothAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/org/bluez/agent"));
}

void BluetoothAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    KyInfo();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(device && ptr)
    {
        d = ptr->getDevPtr(device->address());
    }

    if(d)
    {
        d->requestPinCode(request);
    }
    else
    {
        request.reject();
    }
}

void BluetoothAgent::displayPinCode(BluezQt::DevicePtr device, const QString &pinCode)
{
    KyInfo();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(device && ptr)
    {
        d = ptr->getDevPtr(device->address());
    }

    if(d)
    {
        d->displayPinCode(pinCode);
    }

}

void BluetoothAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    KyInfo();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(device && ptr)
    {
        d = ptr->getDevPtr(device->address());
    }

    if(d)
    {
        d->requestPasskey(request);
    }
    else
    {
        request.reject();
    }
}

void BluetoothAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered)
{
    KyInfo();
    this->displayPinCode(device, passkey);
}

void BluetoothAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    KyInfo() << device->name() << passkey << device->isConnected() << device->isPaired();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(device && ptr)
    {
        d = ptr->getDevPtr(device->address());
    }

    if(d)
    {
        d->requestConfirmation(passkey, request);
    }
    else
    {
        request.reject();
    }
}

void BluetoothAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    KyInfo() << device->name();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(device && ptr)
    {
        d = ptr->getDevPtr(device->address());
    }


    if(d)
    {
        d->requestAuthorization(request);
    }
    else
    {
        request.reject();
    }
}

void BluetoothAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    KyInfo() << device->name();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(device && ptr)
    {
        d = ptr->getDevPtr(device->address());
    }

    if(d)
    {
        d->authorizeService(uuid, request);
    }
    else
    {
        request.reject();
    }
}

void BluetoothAgent::cancel()
{
    KyInfo();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(ptr)
    {
        ptr->clearPinCode();
    }
}

void BluetoothAgent::release()
{
    KyInfo();
}

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

#include "bluetoothobexagent.h"
#include "filesess.h"

BluetoothObexAgent::BluetoothObexAgent(QObject *parent):ObexAgent(parent)
{
    KyInfo();
}

BluetoothObexAgent::~BluetoothObexAgent()
{
    KyInfo();
}


QDBusObjectPath BluetoothObexAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/org/bluez/obex/Agent1"));
}


void BluetoothObexAgent::authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    KyInfo();
    if(nullptr == transfer || nullptr == session)
    {
        request.reject();
    }
    m_session = session;
    FILESESSMNG::getInstance()->receiveFiles(transfer, session, request);
}


void BluetoothObexAgent::cancel()
{
    FILESESSMNG::getInstance()->sessionCanceled(m_session);
    KyInfo();
}

void BluetoothObexAgent::release()
{
    KyInfo();
}


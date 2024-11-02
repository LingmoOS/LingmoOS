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

#ifndef BLUETOOTHOBEXAGENT_H
#define BLUETOOTHOBEXAGENT_H

#include <KF5/BluezQt/bluezqt/obexagent.h>
#include <KF5/BluezQt/bluezqt/obexfiletransfer.h>
#include <KF5/BluezQt/bluezqt/obexfiletransferentry.h>
#include <KF5/BluezQt/bluezqt/obextransfer.h>
#include <KF5/BluezQt/bluezqt/obexsession.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <KF5/BluezQt/bluezqt/request.h>

#include <QDBusObjectPath>
#include <QMetaObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QDebug>
#include <QMetaEnum>

#include "lingmo-log4qt.h"
#include "CSingleton.h"

class BluetoothObexAgent : public BluezQt::ObexAgent
{
    Q_OBJECT
public:
    explicit BluetoothObexAgent(QObject *parent = nullptr);
    ~BluetoothObexAgent();

    void authorizePush (BluezQt::ObexTransferPtr transfer,BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request);
    QDBusObjectPath objectPath() const override;
    void cancel ();
    void release ();

private:
    BluezQt::ObexSessionPtr m_session = nullptr;

    friend class SingleTon<BluetoothObexAgent>;
};

typedef SingleTon<BluetoothObexAgent>  BLUETOOTHOBEXAGENT;

#endif // BLUETOOTHOBEXAGENT_H

// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netcheck_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class NetcheckAdaptor
 */

NetcheckAdaptor::NetcheckAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

NetcheckAdaptor::~NetcheckAdaptor()
{
    // destructor
}

bool NetcheckAdaptor::availabled() const
{
    // get the value of property Availabled
    return qvariant_cast< bool >(parent()->property("Availabled"));
}

void NetcheckAdaptor::ExitApp()
{
    // handle method call com.deepin.pc.manager.netcheck.ExitApp
    QMetaObject::invokeMethod(parent(), "ExitApp");
}

bool NetcheckAdaptor::GetNetCheckStatus()
{
    // handle method call com.deepin.pc.manager.netcheck.GetNetCheckStatus
    bool status;
    QMetaObject::invokeMethod(parent(), "GetNetCheckStatus", Q_RETURN_ARG(bool, status));
    return status;
}

void NetcheckAdaptor::SetNetCheckStatus(bool status)
{
    // handle method call com.deepin.pc.manager.netcheck.SetNetCheckStatus
    QMetaObject::invokeMethod(parent(), "SetNetCheckStatus", Q_ARG(bool, status));
}

void NetcheckAdaptor::StartApp()
{
    // handle method call com.deepin.pc.manager.netcheck.StartApp
    QMetaObject::invokeMethod(parent(), "StartApp");
}

void NetcheckAdaptor::StartCheckNetConn(const QString &url)
{
    // handle method call com.deepin.pc.manager.netcheck.StartCheckNetConn
    QMetaObject::invokeMethod(parent(), "StartCheckNetConn", Q_ARG(QString, url));
}

void NetcheckAdaptor::StartCheckNetConnSetting()
{
    // handle method call com.deepin.pc.manager.netcheck.StartCheckNetConnSetting
    QMetaObject::invokeMethod(parent(), "StartCheckNetConnSetting");
}

void NetcheckAdaptor::StartCheckNetDHCP()
{
    // handle method call com.deepin.pc.manager.netcheck.StartCheckNetDHCP
    QMetaObject::invokeMethod(parent(), "StartCheckNetDHCP");
}

void NetcheckAdaptor::StartCheckNetDNS()
{
    // handle method call com.deepin.pc.manager.netcheck.StartCheckNetDNS
    QMetaObject::invokeMethod(parent(), "StartCheckNetDNS");
}

void NetcheckAdaptor::StartCheckNetDevice()
{
    // handle method call com.deepin.pc.manager.netcheck.StartCheckNetDevice
    QMetaObject::invokeMethod(parent(), "StartCheckNetDevice");
}

void NetcheckAdaptor::StartCheckNetHost()
{
    // handle method call com.deepin.pc.manager.netcheck.StartCheckNetHost
    QMetaObject::invokeMethod(parent(), "StartCheckNetHost");
}


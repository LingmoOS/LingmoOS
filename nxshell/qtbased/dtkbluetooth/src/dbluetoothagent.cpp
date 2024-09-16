// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothagent.h"

DBLUETOOTH_BEGIN_NAMESPACE

DAgent::DAgent(QObject *parent)
    : QObject(parent)
{
}

DAgent::Capability DAgent::capability() const
{
    return DisplayYesNo;
}

void DAgent::requestPinCode(QSharedPointer<DDevice>, const DRequest<QString> &request)
{
    request.cancel();
}

void DAgent::displayPinCode(QSharedPointer<DDevice>, const QString &) {}

void DAgent::requestPasskey(QSharedPointer<DDevice>, const DRequest<quint32> &request)
{
    request.cancel();
}

void DAgent::displayPasskey(QSharedPointer<DDevice>, const QString &, const QString &) {}

void DAgent::requestConfirmation(QSharedPointer<DDevice>, const QString &, const DRequest<> &request)
{
    request.cancel();
}

void DAgent::requestAuthorization(QSharedPointer<DDevice>, const DRequest<> &request)
{
    request.cancel();
}

void DAgent::authorizeService(QSharedPointer<DDevice>, const QString &, const DRequest<> &request)
{
    request.cancel();
}

void DAgent::cancel() {}

void DAgent::release() {}

DBLUETOOTH_END_NAMESPACE

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothobexagent.h"

DBLUETOOTH_BEGIN_NAMESPACE

DObexAgent::DObexAgent(QObject *parent)
    : QObject(parent)
{
}

void DObexAgent::authorizePush(QSharedPointer<DObexTransfer>, QSharedPointer<DObexSession>, const DRequest<QString> &request)
{
    request.cancel();
}

void DObexAgent::cancel() {}

void DObexAgent::release() {}

DBLUETOOTH_END_NAMESPACE

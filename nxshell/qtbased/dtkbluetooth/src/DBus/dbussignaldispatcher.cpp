// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbussignaldispatcher.h"

DBLUETOOTH_BEGIN_NAMESPACE

QDBusPendingReply<ObjectMap> DBusSignalDispatcher::getManagedObjects() const
{
    return source->getManagedObjects();
}

DBLUETOOTH_END_NAMESPACE

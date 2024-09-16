// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dunioniddbus.h"
#include "commondef.h"
#include <QLoggingCategory>

DUnionIDDbus::DUnionIDDbus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    auto reply = this->SwitcherDump();
    reply.waitForFinished();
    if (!reply.isValid()) {
        qCWarning(ServiceLogger) << "Error connecting remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
        qCWarning(ServiceLogger) << reply.error();
    } else {
        qCInfo(ServiceLogger) << "connected remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
        qCInfo(ServiceLogger) << "switcher dump" << reply.value();
    }
}

DUnionIDDbus::~DUnionIDDbus()
{

}

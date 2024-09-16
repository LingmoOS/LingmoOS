// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "systempowerinterface.h"

DPOWER_BEGIN_NAMESPACE
SystemPowerInterface::SystemPowerInterface(QObject *parent)
    : QObject(parent)
{
    const QString &Service = QStringLiteral("com.deepin.system.Power");
    const QString &Path = QStringLiteral("/com/deepin/system/Power");
    const QString &Interface = QStringLiteral("com.deepin.system.Power");

    m_inter.reset(new DDBusInterface(Service, Path, Interface, QDBusConnection::systemBus(), this));
}

DPOWER_END_NAMESPACE

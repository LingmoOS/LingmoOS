// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionpath.h"

bool SessionPath::operator ==(const SessionPath &other)
{
    return (out0 == other.out0) && (path == other.path);
}

bool SessionPath::operator !=(const SessionPath &other)
{
    return (out0 != other.out0) || (path != other.path);
}

QDBusArgument &operator<<(QDBusArgument &arg, const SessionPath &other)
{
    arg.beginStructure();
    arg << other.out0 << other.path;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, SessionPath &other)
{
    arg.beginStructure();
    arg >> other.out0 >> other.path;
    arg.endStructure();

    return arg;
}

void registerSessionPathMetaType()
{
    qRegisterMetaType<SessionPath>("SessionPath");
    qDBusRegisterMetaType<SessionPath>();
}

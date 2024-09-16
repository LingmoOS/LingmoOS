// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderupdatestate.h"

bool DefenderUpdateState::operator!=(const DefenderUpdateState &updateState)
{
    return updateState.info.time != info.time || updateState.info.version != info.version || updateState.info.description != info.description || updateState.progress != progress;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderUpdateState &updateState)
{
    argument.beginStructure();
    argument << updateState.progress << updateState.info.version << updateState.info.description << updateState.info.time;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderUpdateState &updateState)
{
    argument.beginStructure();
    argument >> updateState.progress >> updateState.info.version >> updateState.info.description >> updateState.info.time;
    argument.endStructure();
    return argument;
}

void registerDefenderUpdateStateMetaType()
{
    qRegisterMetaType<DefenderUpdateState>("DefenderUpdateState");
    qDBusRegisterMetaType<DefenderUpdateState>();
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderappflow.h"

//DefenderAppFlowList注册
QDBusArgument &operator<<(QDBusArgument &argument, const DefenderAppFlow &appFlow)
{
    argument.beginStructure();
    argument << appFlow.id << appFlow.name << appFlow.sname << appFlow.timeymdh << appFlow.downloaded
             << appFlow.uploaded << appFlow.apptotal;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderAppFlow &appFlow)
{
    argument.beginStructure();
    argument >> appFlow.id >>appFlow.name >> appFlow.sname >> appFlow.timeymdh >> appFlow.downloaded
             >> appFlow.uploaded >> appFlow.apptotal;
    argument.endStructure();
    return argument;
}

void registerDefenderAppFlowMetaType()
{
    qRegisterMetaType<DefenderAppFlow>("DefenderAppFlow");
    qDBusRegisterMetaType<DefenderAppFlow>();
    qRegisterMetaType<DefenderAppFlowList>("DefenderAppFlowList");
    qDBusRegisterMetaType<DefenderAppFlowList>();
}

bool DefenderAppFlow::operator!=(const DefenderAppFlow &appFlow)
{
    return appFlow.id != this->id;
}

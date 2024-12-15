// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderdissection.h"

bool DefenderDissection::operator!=(const DefenderDissection &dissectionInfo)
{
    return dissectionInfo.taskId != taskId || dissectionInfo.progress != progress || dissectionInfo.fileInfo.type != fileInfo.type || dissectionInfo.fileInfo.description != fileInfo.description;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderDissection &dissectionInfo)
{
    argument.beginStructure();
    argument << dissectionInfo.taskId << dissectionInfo.progress << dissectionInfo.fileInfo.type << dissectionInfo.fileInfo.description;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderDissection &dissectionInfo)
{
    argument.beginStructure();
    argument >> dissectionInfo.taskId >> dissectionInfo.progress >> dissectionInfo.fileInfo.type >> dissectionInfo.fileInfo.description;
    argument.endStructure();
    return argument;
}

void registerDefenderDissectionMetaType()
{
    qRegisterMetaType<DefenderDissection>("DefenderDissection");
    qDBusRegisterMetaType<DefenderDissection>();
}

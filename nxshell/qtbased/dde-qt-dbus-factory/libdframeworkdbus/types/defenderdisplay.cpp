// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderdisplay.h"

bool DefenderDisplay::operator!=(const DefenderDisplay &display)
{
    return display.pid != pid || display.icon != icon || display.path != path || display.download != download ||
            display.name != name || display.update != update || display.total != total || display.pidname != pidname
            || display.status != status || display.totalupdate != totalupdate || display.totaldownload != totaldownload
            || display.totalflow != totalflow;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderDisplay &defenderdisplay)
{
    argument.beginStructure();
    argument << defenderdisplay.pid << defenderdisplay.icon << defenderdisplay.name << defenderdisplay.path
             << defenderdisplay.status << defenderdisplay.pidname << defenderdisplay.total
             << defenderdisplay.update << defenderdisplay.download << defenderdisplay.totalupdate
             << defenderdisplay.totaldownload << defenderdisplay.totalflow;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderDisplay &defenderdisplay)
{
    argument.beginStructure();
    argument >> defenderdisplay.pid >> defenderdisplay.icon >> defenderdisplay.name >> defenderdisplay.path
             >> defenderdisplay.status >> defenderdisplay.pidname >> defenderdisplay.total
             >> defenderdisplay.update >> defenderdisplay.download >> defenderdisplay.totalupdate
             >> defenderdisplay.totaldownload >> defenderdisplay.totalflow;
    argument.endStructure();
    return argument;
}

void registerDefenderDisplayMetaType()
{
    qRegisterMetaType<DefenderDisplayList>("DefenderDisplayList");
    qDBusRegisterMetaType<DefenderDisplayList>();
    qRegisterMetaType<DefenderDisplay>("DefenderDisplay");
    qDBusRegisterMetaType<DefenderDisplay>();
}

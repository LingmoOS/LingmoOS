// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderprocesslist.h"

void registerDefenderProcessListMetaType()
{
    qRegisterMetaType<DefenderProcessList>("DefenderProcessList");
    qDBusRegisterMetaType<DefenderProcessList>();

    qRegisterMetaType<QList<QString>>("QList<QString>");
    qDBusRegisterMetaType<QList<QString>>();
}

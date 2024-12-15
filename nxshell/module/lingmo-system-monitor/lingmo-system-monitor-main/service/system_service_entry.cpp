// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_service_entry.h"

SystemServiceEntry::SystemServiceEntry()
    : data(new SystemServiceEntryData)
{
}
SystemServiceEntry::SystemServiceEntry(const QString &id,
                                       const QString &sname,
                                       const QString &loadState,
                                       const QString &activeState,
                                       const QString &subState,
                                       const QString &state,
                                       const QString &startupType,
                                       const QString &unitObjectPath,
                                       const QString &description,
                                       quint32 mainPID,
                                       bool canReload,
                                       bool canStart,
                                       bool canStop)
    : data(new SystemServiceEntryData)
{
    setId(id);
    setSName(sname);
    setLoadState(loadState);
    setActiveState(activeState);
    setSubState(subState);
    setState(state);
    setStartupType(startupType);
    setUnitObjectPath(unitObjectPath);
    setDescription(description);
    setMainPID(mainPID);
    setCanReload(canReload);
    setCanStart(canStart);
    setCanStop(canStop);
}

SystemServiceEntry::SystemServiceEntry(const SystemServiceEntry &rhs)
    : data(rhs.data)
{
}

SystemServiceEntry &SystemServiceEntry::operator=(const SystemServiceEntry &rhs)
{
    if (this != &rhs)
        data.operator = (rhs.data);
    return *this;
}

SystemServiceEntry::~SystemServiceEntry() {}

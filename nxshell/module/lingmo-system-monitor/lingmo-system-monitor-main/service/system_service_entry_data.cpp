// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_service_entry_data.h"

SystemServiceEntryData::SystemServiceEntryData() {}

SystemServiceEntryData::SystemServiceEntryData(const SystemServiceEntryData &rhs)
    : QSharedData(rhs)
    , m_id(rhs.m_id)
    , m_sname(rhs.m_sname)
    , m_loadState(rhs.m_loadState)
    , m_activeState(rhs.m_activeState)
    , m_subState(rhs.m_subState)
    , m_state(rhs.m_state)
    , m_startupType(rhs.m_startupType)
    , m_unitObjectPath(rhs.m_unitObjectPath)
    , m_description(rhs.m_description)
    , m_mainPID(rhs.m_mainPID)
    , m_canReload(rhs.m_canReload)
    , m_canStart(rhs.m_canStart)
    , m_canStop(rhs.m_canStop)
{
}

SystemServiceEntryData &SystemServiceEntryData::operator=(const SystemServiceEntryData &rhs)
{
    if (this != &rhs) {
        m_id.operator = (rhs.m_id);
        m_sname.operator = (rhs.m_sname);
        m_loadState.operator = (rhs.m_loadState);
        m_activeState.operator = (rhs.m_activeState);
        m_subState.operator = (rhs.m_subState);
        m_state.operator = (rhs.m_state);
        m_startupType.operator = (rhs.m_startupType);
        m_unitObjectPath = rhs.m_unitObjectPath;
        m_description.operator = (rhs.m_description);
        m_mainPID = rhs.m_mainPID;
        m_canReload = rhs.m_canReload;
        m_canStart = rhs.m_canStart;
        m_canStop = rhs.m_canStop;
    }
    return *this;
}

SystemServiceEntryData::~SystemServiceEntryData() {}

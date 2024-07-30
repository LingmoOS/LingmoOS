/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QMetaType>

#include "process.h"

namespace KSysGuardProcess
{
Q_NAMESPACE

using ProcessStatus = KSysGuard::Process::ProcessStatus;
Q_ENUM_NS(ProcessStatus)

using IoPriorityClass = KSysGuard::Process::IoPriorityClass;
Q_ENUM_NS(IoPriorityClass)

using Scheduler = KSysGuard::Process::Scheduler;
Q_ENUM_NS(Scheduler)
}

Q_DECLARE_METATYPE(KSysGuard::Process::ProcessStatus)
Q_DECLARE_METATYPE(KSysGuard::Process::IoPriorityClass)
Q_DECLARE_METATYPE(KSysGuard::Process::Scheduler)

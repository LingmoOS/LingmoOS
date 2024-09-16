// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/accessibledefine.h"

#include "memorywidget.h"

// 添加accessible
SET_FORM_ACCESSIBLE(MemoryWidget, "MemoryWidget")

QAccessibleInterface *memoryMonitorAccessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    USE_ACCESSIBLE(classname, MemoryWidget)

    return interface;
}

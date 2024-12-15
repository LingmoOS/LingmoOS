// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmonitorservice.h"

static SystemMonitorService *monitorService = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    (void)data;
    monitorService = new SystemMonitorService(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    (void)name;
    (void)data;
    monitorService->deleteLater();
    monitorService = nullptr;
    return 0;
}

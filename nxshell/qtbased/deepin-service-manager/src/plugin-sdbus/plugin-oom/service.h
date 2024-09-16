// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OOM_SERVICE_H
#define OOM_SERVICE_H

#include "3rdparty/earlyoom/globals.h"
#include "3rdparty/earlyoom/meminfo.h"

#include <systemd/sd-bus-vtable.h>

DLL_LOCAL void service_set_dbus(sd_bus *bus);
DLL_LOCAL void emit_killed(const procinfo_t *victim);
DLL_LOCAL int dbus_add_object(void);

#endif // OOM_SERVICE_H

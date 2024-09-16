// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "service.h"

#include "3rdparty/earlyoom/msg.h"

#include <systemd/sd-bus-vtable.h>

#include <errno.h>
#include <stdio.h>

static sd_bus *dbus = NULL;
static char *dbus_path = "/org/deepin/oom1";
static char *dbus_interface = "org.deepin.oom1";
static const sd_bus_vtable oom_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_SIGNAL("Killed", "iisit", 0),
    SD_BUS_VTABLE_END
};

void service_set_dbus(sd_bus *bus)
{
    dbus = bus;
}

int dbus_add_object()
{
    sd_bus_slot *slot = NULL;
    int ret = sd_bus_add_object_vtable(dbus, &slot, dbus_path, dbus_interface, oom_vtable, NULL);
    if (ret < 0) {
        warn("Failed to issue method call: %s\n", strerror(-ret));
        return -1;
    }
    debug("register oom plugin success\n");
    return 0;
}

void emit_killed(const procinfo_t *victim)
{
    sd_bus_emit_signal(dbus,
                       dbus_path,
                       dbus_interface,
                       "Killed",
                       "iisit",
                       victim->pid,
                       victim->uid,
                       victim->name,
                       victim->badness,
                       victim->VmRSSkiB);
}

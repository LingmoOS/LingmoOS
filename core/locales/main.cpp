// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localesservice.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/logging.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Lingmo::Base::EventLoop loop;
    static const Lingmo::Base::LogCategory log("lingmo.locales");

    LocalesService service;

    Lingmo::Base::DBus::Connection *bus = Lingmo::Base::DBus::Connection::system(&loop);
    if (!bus || !service.exportTo(bus) || !bus->requestName("org.lingmo.Locales")) {
        LM_WARNING(log) << "lingmo-locales: cannot register on the system bus";
        return 1;
    }

    service.onIdle = [&loop] {
        loop.quit();
    };

    return loop.run();
}

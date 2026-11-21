// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "privacyservice.h"

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/logging.h"

// The udev rules test the files in here; systemd's StateDirectory creates it.
static constexpr char kStateDirectory[] = "/var/lib/lingmo-privacy";

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Lingmo::Base::EventLoop loop;
    static const Lingmo::Base::LogCategory log("lingmo.privacy");

    PrivacyService service(kStateDirectory);

    Lingmo::Base::DBus::Connection *bus = Lingmo::Base::DBus::Connection::system(&loop);
    if (!bus || !service.exportTo(bus) || !bus->requestName("org.lingmo.Privacy")) {
        LM_WARNING(log) << "lingmo-privacy: cannot register on the system bus";
        return 1;
    }

    service.onIdle = [&loop] {
        loop.quit();
    };

    return loop.run();
}

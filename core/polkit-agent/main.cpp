// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "polkitagentlistener.h"

#include "base/dbus.h"
#include "base/i18n.h"
#include "ui/application.h"

int main(int argc, char *argv[])
{
    Lingmo::UI::Application app(argc, argv);
    app.setApplicationId("lingmo-polkit-agent");
    Lingmo::I18n::setDomain("lingmo-core");

    PolKitAgentListener listener(Lingmo::Base::DBus::Connection::system());

    if (!listener.registerAgent("/org/lingmo/PolicyKit1/AuthenticationAgent"))
        return -1;

    return app.run();
}

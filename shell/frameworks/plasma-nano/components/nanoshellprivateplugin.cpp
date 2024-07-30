/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "nanoshellprivateplugin.h"
#include "fullscreenoverlay.h"
#include "startupnotifier.h"

void PlasmaMiniShellPrivatePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.nanoshell"));

    qmlRegisterType<FullScreenOverlay>(uri, 2, 0, "FullScreenOverlay");
    qmlRegisterSingletonType<StartupNotifier>(uri, 2, 0, "StartupNotifier", [](QQmlEngine *, QJSEngine *) {
        return new StartupNotifier;
    });
}

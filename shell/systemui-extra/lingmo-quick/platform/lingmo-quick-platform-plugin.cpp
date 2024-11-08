/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "lingmo-quick-platform-plugin.h"
#include "window-type.h"
#include "windows/lingmo-window.h"
#include "lingmo/lingmo-theme-proxy.h"
#include "lingmo/settings.h"
#include "lingmo/app-launcher.h"
#include "window-manager.h"

void LingmoUIQuickPlatformPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    qmlRegisterModule(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterSingletonType<LingmoUIQuick::Settings>(uri, 1, 0, "Settings", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return LingmoUIQuick::Settings::instance();
    });
    qmlRegisterSingletonType<LingmoUIQuick::AppLauncher>(uri, 1, 0, "AppLauncher", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return LingmoUIQuick::AppLauncher::instance();
    });
    qmlRegisterUncreatableType<LingmoUIQuick::Theme>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Theme", "Accessing Theme through Attached Property.");
    qmlRegisterUncreatableType<LingmoUIQuick::WindowType>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowType",
                                                      "WindowType is a read-only interface used to access enumeration properties.");

    qmlRegisterExtendedType<LingmoUIQuick::LINGMOWindow, LingmoUIQuick::LINGMOWindowExtension>(uri, 1, 0, "LINGMOWindow");
    qmlRegisterSingletonType<LingmoUIQuick::WindowManager>(uri, 1, 0, "WindowManager", [] (QQmlEngine *, QJSEngine *) -> QObject* {
    return LingmoUIQuick::WindowManager::self();
});
}

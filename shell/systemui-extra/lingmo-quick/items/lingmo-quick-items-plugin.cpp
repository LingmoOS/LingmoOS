/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "lingmo-quick-items-plugin.h"
#include "margin.h"
#include "types.h"
#include "menu.h"
#include "menu-item.h"
#include "theme-icon.h"
#include "tooltip-proxy.h"
#include "icon.h"
#include "icon-provider.h"
#include "color-mixer.h"
#include "content-window.h"
#include "windows/dialog.h"
#include "tooltip.h"
#include "action-extension.h"
#include "window-blur-behind.h"

#include <QtQml>
#include <QAction>
#include <QScreen>

void LingmoUIQuickItemsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    qmlRegisterModule(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);

    qRegisterMetaType<QScreen*>();
    qRegisterMetaType<LingmoUIQuick::Types::Pos>();
    qRegisterMetaType<LingmoUIQuick::Dialog::PopupLocation>();

    // value types
    qmlRegisterType<LingmoUIQuick::Icon>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Icon");
    qmlRegisterType<LingmoUIQuick::Menu>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Menu");
    qmlRegisterType<LingmoUIQuick::MenuItem>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "MenuItem");
    qmlRegisterType<LingmoUIQuick::ThemeIcon>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ThemeIcon");
    qmlRegisterType<LingmoUIQuick::ColorMix>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ColorMix");

    qmlRegisterUncreatableType<LingmoUIQuick::ToolTipAttached>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ToolTip", "Proxy tool pointing to QTooltip, accessed only through Attached property \"ToolTip\".");
    qmlRegisterUncreatableType<LingmoUIQuick::Types>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Types", "Types is a read-only interface used to access enumeration properties.");
    qmlRegisterUncreatableType<LingmoUIQuick::Margin>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Margin", "Margin is a read-only attribute that can be accessed from the ContentWindow.");

    qmlRegisterType<LingmoUIQuick::Dialog>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Dialog");
    qmlRegisterType<LingmoUIQuick::Tooltip>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "Tooltip");
    qmlRegisterType<LingmoUIQuick::ContentWindow>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "ContentWindow");

    // 注册QAction并为它扩展属性
    qmlRegisterExtendedType<QAction, LingmoUIQuick::ActionExtension>(uri, 1, 0, "Action");

    qmlRegisterType<LingmoUIQuick::RectRegion>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "RectRegion");
    qmlRegisterType<LingmoUIQuick::WindowBlurBehind>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, "WindowBlurBehind");

#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    qmlRegisterRevision<QWindow, 3>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterRevision<QQuickWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#else
    #if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    qmlRegisterRevision<QWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterRevision<QQuickWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    qmlRegisterRevision<QWindow, 2>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    qmlRegisterRevision<QQuickWindow, 1>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#else
    qmlRegisterRevision<QWindow, 1>(uri, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
#endif
#endif
#endif
}

void LingmoUIQuickItemsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    engine->addImageProvider("theme", new LingmoUIQuick::IconProvider());
}

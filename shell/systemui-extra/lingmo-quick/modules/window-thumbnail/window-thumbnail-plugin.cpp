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
#include "window-thumbnail-plugin.h"
#include <QQmlContext>
#include <QQmlEngine>

#include "pipewire-source-item.h"
#include "screen-casting.h"
#include "screen-casting-request.h"
#include "x-window-thumbnail.h"
#include "window-thumbnail-config.h"
#include "window-thumbnail-mpris-model.h"
#include "kywlcom-window-thumbnail-item.h"

void WindowThumbnailPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    qmlRegisterType<PipeWireSourceItem>(uri, 1, 0, "PipeWireSourceItem");
    qmlRegisterType<ScreenCastingRequest>(uri, 1, 0, "ScreenCastingRequest");
    qmlRegisterType<XWindowThumbnail>(uri, 1, 0, "XWindowThumbnail");
    qmlRegisterType<WindowThumbnailMprisModel>(uri, 1, 0, "WindowThumbnailMprisModel");
    qmlRegisterType<KywlcomWindowThumbnailItem>(uri, 1, 0, "KywlcomWindowThumbnailItem");

    qmlRegisterUncreatableType<Screencasting>(uri, 1, 0, "Screencasting", "Only enumeration variables are required");
    qmlRegisterUncreatableType<MprisProperties>(uri, 1, 0, "MprisProperties", "Only enumeration variables are required");
    qRegisterMetaType<MprisProperties::Operations>("MprisProperties::Operations");
    qRegisterMetaType<MprisProperties::Properties>("MprisProperties::Properties");
}

void WindowThumbnailPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));
    engine->rootContext()->setContextProperty("windowThumbnailConfig", new WindowThumbnailConfig);
}

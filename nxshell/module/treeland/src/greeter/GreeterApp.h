/***************************************************************************
 * Copyright (c) 2015-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ***************************************************************************/

#ifndef GREETERAPP_H
#define GREETERAPP_H

#include "GreeterProxy.h"
#include "LogoProvider.h"
#include "SessionModel.h"
#include "UserModel.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QTranslator>
#include <qqml.h>
#include <qqmlextensionplugin.h>
#include <qscopedpointer.h>

class GreeterExtensionPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)
public:
    GreeterExtensionPlugin()
        : QQmlEngineExtensionPlugin()
    {
        Q_INIT_RESOURCE(greeter_assets);
        qmlRegisterType<SessionModel>("TreeLand.Greeter", 1, 0, "SessionModel");
        qmlRegisterType<UserModel>("TreeLand.Greeter", 1, 0, "UserModel");
        qmlRegisterType<GreeterProxy>("TreeLand.Greeter", 1, 0, "Proxy");
        qmlRegisterType<logoProvider>("TreeLand.Greeter", 1, 0, "LogoProvider");
    }

    void initializeEngine([[maybe_unused]] QQmlEngine *engine,
                          [[maybe_unused]] const char *uri) final
    {
    }

    ~GreeterExtensionPlugin() { Q_CLEANUP_RESOURCE(greeter_assets); }
};

#endif // GREETERAPP_H

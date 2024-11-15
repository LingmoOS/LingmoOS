#include "qmlplugins.h"
#include "networkmodel.h"
#include "networkmodelitem.h"
#include "activeconnection.h"
#include "appletproxymodel.h"
#include "wiressitemsettings.h"
#include "identitymodel.h"
#include "handler.h"
#include "enabledconnections.h"
#include "enums.h"
#include "wifisettings.h"
#include "configuration.h"

#include <QQmlEngine>

void QmlPlugins::registerTypes(const char* uri)
{
    qmlRegisterUncreatableType<NetworkModelItem>(uri, 1, 0, "NetworkModelItem",
                                                QLatin1String("Cannot instantiate NetworkModelItem"));
    qmlRegisterType<ActiveConnection>(uri, 1, 0, "ActiveConnection");
    qmlRegisterType<AppletProxyModel>(uri, 1, 0, "AppletProxyModel");
    qmlRegisterType<NetworkModel>(uri, 1, 0, "NetworkModel");
    qmlRegisterType<WirelessItemSettings>(uri, 1, 0, "WirelessItemSettings");
    qmlRegisterType<IdentityModel>(uri, 1, 0, "IdentityModel");
    qmlRegisterType<Handler>(uri, 1, 0, "Handler");
    qmlRegisterType<EnabledConnections>(uri, 1, 0, "EnabledConnections");
    qmlRegisterType<WifiSettings>(uri, 1, 0, "WifiSettings");
    qmlRegisterType<Configuration>(uri, 1, 0, "Configuration");
    qmlRegisterUncreatableType<Enums>(uri, 1, 0, "Enums", "You cannot create Enums on yourself");
}

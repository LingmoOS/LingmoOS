#include "dbuspropertiesinterface.h"

DBusPropertiesInterface::DBusPropertiesInterface(const QString &service,
                                                 const QString &path,
                                                 const QDBusConnection &connection,
                                                 QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    setTimeout(500);
}

DBusPropertiesInterface::~DBusPropertiesInterface()
{
}

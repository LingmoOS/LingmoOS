#include "timedated_interface.h"

/*
 * Implementation of interface class OrgFreedesktopTimedate1Interface
 */

OrgFreedesktopTimedate1Interface::OrgFreedesktopTimedate1Interface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgFreedesktopTimedate1Interface::~OrgFreedesktopTimedate1Interface()
{
}

#ifndef SYNC_DBUS_SERVER_H
#define SYNC_DBUS_SERVER_H

#include "gdbus/libsyncconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int monitoring_requires_synchronized_configuration();

    gboolean init_gdbus_server(GBusType type);

#ifdef __cplusplus
}
#endif

#endif // SYNC_DBUS_SERVER_H
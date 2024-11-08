#include "sync-dbus-server.h"

int main(int argc, char const *argv[])
{
    monitoring_requires_synchronized_configuration();
    init_gdbus_server(G_BUS_TYPE_SESSION);
    return 0;
}
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include <KDEDModule>
#include <KPluginFactory>
#include <QCoreApplication>

#include "dbusobjectmanagerserver.h"
#include "device.h"
#include "smartctl.h"
#include "smartmonitor.h"
#include "smartnotifier.h"
#include "soliddevicenotifier.h"

#ifdef WITH_SIMULATION
#include "simulationctl.h"
#include "simulationdevicenotifier.h"
#endif

#ifdef WITH_SIMULATION
static bool isSimulation()
{
    return qEnvironmentVariableIntValue("PLASMA_DISKS_SIMULATION") == 1;
}
#endif

template<class... Args>
static std::unique_ptr<AbstractSMARTCtl> make_unique_smartctl(Args &&...args)
{
#ifdef WITH_SIMULATION
    if (isSimulation()) {
        return std::make_unique<SimulationCtl>(std::forward<Args>(args)...);
    }
#endif
    return std::make_unique<SMARTCtl>(std::forward<Args>(args)...);
}

template<class... Args>
static std::unique_ptr<DeviceNotifier> make_unique_devicenotifier(Args &&...args)
{
#ifdef WITH_SIMULATION
    if (isSimulation()) {
        return std::make_unique<SimulationDeviceNotifier>(std::forward<Args>(args)...);
    }
#endif
    return std::make_unique<SolidDeviceNotifier>(std::forward<Args>(args)...);
}

class SMARTModule : public KDEDModule
{
    Q_OBJECT
public:
    explicit SMARTModule(QObject *parent, const QVariantList &args)
        : KDEDModule(parent)
    {
#ifdef WITH_SIMULATION
        Q_INIT_RESOURCE(simulation);
#endif
        Q_UNUSED(args);
        connect(&m_monitor, &SMARTMonitor::deviceAdded, this, [this](Device *device) {
            dbusDeviceServer.serve(device);
        });
        connect(&m_monitor, &SMARTMonitor::deviceRemoved, &dbusDeviceServer, [this](Device *device) {
            dbusDeviceServer.unserve(device);
        });
        m_monitor.start();
    }

private:
    SMARTMonitor m_monitor{make_unique_smartctl(), make_unique_devicenotifier()};
    SMARTNotifier m_notifier{&m_monitor};
    KDBusObjectManagerServer dbusDeviceServer;
};

K_PLUGIN_FACTORY_WITH_JSON(SMARTModuleFactory, "smart.json", registerPlugin<SMARTModule>();)

#include "main.moc"

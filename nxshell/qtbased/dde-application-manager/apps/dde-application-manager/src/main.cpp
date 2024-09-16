// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "global.h"
#include <QDBusConnection>
#include <QCoreApplication>
#include <QDir>
#include "dbus/applicationmanager1service.h"
#include "cgroupsidentifier.h"
#include "applicationmanagerstorage.h"
#include <chrono>
#include <iostream>

Q_LOGGING_CATEGORY(DDEAMProf, "dde.am.prof", QtInfoMsg)

namespace {
void registerComplexDbusType()
{
    qRegisterMetaType<ObjectInterfaceMap>();
    qDBusRegisterMetaType<ObjectInterfaceMap>();
    qRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<QStringMap>();
    qRegisterMetaType<QStringMap>();
    qRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<QDBusObjectPath>();
}
}  // namespace

int main(int argc, char *argv[])
{
#ifdef PROFILING_MODE
    auto start = std::chrono::high_resolution_clock::now();
#endif
    QCoreApplication app{argc, argv};

    auto &bus = ApplicationManager1DBus::instance();
    bus.initGlobalServerBus(DBusType::Session);
    bus.setDestBus();
    auto &AMBus = bus.globalServerBus();

    registerComplexDbusType();
    auto storageDir = getXDGDataHome() + QDir::separator() + "deepin" + QDir::separator() + "ApplicationManager";
    auto storage = ApplicationManager1Storage::createApplicationManager1Storage(storageDir);

    ApplicationManager1Service AMService{std::make_unique<CGroupsIdentifier>(), storage};
    AMService.initService(AMBus);

#ifdef PROFILING_MODE
    auto end = std::chrono::high_resolution_clock::now();
    qCInfo(DDEAMProf) << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms";
    return 0;
#else
    return QCoreApplication::exec();
#endif
}

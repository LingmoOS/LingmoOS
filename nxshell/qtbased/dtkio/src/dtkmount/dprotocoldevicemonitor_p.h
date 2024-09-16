// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPROTOCOLDEVICEMONITOR_P_H
#define DPROTOCOLDEVICEMONITOR_P_H

#include <DProtocolDeviceMonitor>

#include <gio/gio.h>

#include <QDebug>
#include <QThread>
#include <QCoreApplication>

DMOUNT_BEGIN_NAMESPACE

class DProtocolDeviceMonitorPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DProtocolDeviceMonitor)

public:
    explicit DProtocolDeviceMonitorPrivate(DProtocolDeviceMonitor *qq)
        : QObject { qq }, q_ptr { qq }
    {
        if (QThread::currentThread() != qApp->thread()) {
            qCritical() << "mount: monitor must be initialized in main thread.";
            abort();
        }

        monitor = g_volume_monitor_get();
        if (!monitor) {
            qCritical() << "mount: cannot obtain the GVolumeMonitor object.";
            abort();
        }

        initDevices();
    }

    ~DProtocolDeviceMonitorPrivate() override
    {
        if (monitor)
            g_object_unref(monitor);
        monitor = nullptr;
    }

    void initDevices();
    bool startMonitor();
    bool stopMonitor();

    static void volumeFilter(void *volume, void *data);
    static void mountFilter(void *mount, void *data);
    static bool hasAssociatedDrive(GMount *mount);
    static bool hasAssociatedDrive(GVolume *volume);
    static bool isLocalSourceMount(const QString &mpt);
    static bool isMountOfCurrentUser(const QString &mpt);
    static bool isOrphanMount(GMount *mount);

    static void onMountAdded(GVolumeMonitor *monitor, GMount *mount, gpointer data);
    static void onMountRemoved(GVolumeMonitor *monitor, GMount *mount, gpointer data);
    static void onVolumeAdded(GVolumeMonitor *monitor, GVolume *volume, gpointer data);
    static void onVolumeRemoved(GVolumeMonitor *monitor, GVolume *volume, gpointer data);

public:
    DProtocolDeviceMonitor *q_ptr { nullptr };
    bool isWatching { false };
    QMap<QString, ulong> connections;
    QSet<QString> devUrls;

    GVolumeMonitor *monitor { nullptr };
};

DMOUNT_END_NAMESPACE

#endif   // DPROTOCOLDEVICEMONITOR_P_H

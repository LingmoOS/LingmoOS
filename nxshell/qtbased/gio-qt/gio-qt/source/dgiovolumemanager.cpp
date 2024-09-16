// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgiomount.h"
#include "dgiovolume.h"
#include "dgiodrive.h"
#include "dgiovolumemanager.h"

#include <glibmm/refptr.h>

#include <giomm/init.h>
#include <giomm/volumemonitor.h>

using namespace Gio;

class DGioVolumeManagerPrivate
{
public:
    DGioVolumeManagerPrivate(DGioVolumeManager *qq);
    ~DGioVolumeManagerPrivate();

private:
    Glib::RefPtr<VolumeMonitor> m_gmmVolumeMonitorPtr;

    DGioVolumeManager *q_ptr;

    void slot_mountAdded(const Glib::RefPtr< Mount >& gmmMount);
    void slot_mountRemoved(const Glib::RefPtr< Mount >& gmmMount);
    void slot_mountPreRemoved(const Glib::RefPtr< Mount >& gmmMount);
    void slot_mountChanged(const Glib::RefPtr< Mount >& gmmMount);
    void slot_volumeAdded(const Glib::RefPtr< Volume >& gmmVolume);
    void slot_volumeRemoved(const Glib::RefPtr< Volume >& gmmVolume);
    void slot_volumeChanged(const Glib::RefPtr< Volume >& gmmVolume);
    void slot_driveConnected(const Glib::RefPtr< Drive >& gmmDrive);
    void slot_driveDisconnected(const Glib::RefPtr< Drive >& gmmDrive);
    void slot_driveChanged(const Glib::RefPtr< Drive >& gmmDrive);

    QList<sigc::connection> m_connections;

    Q_DECLARE_PUBLIC(DGioVolumeManager)
};

DGioVolumeManagerPrivate::DGioVolumeManagerPrivate(DGioVolumeManager *qq)
    : q_ptr(qq)
{
    // Do Gio's init or things like Gio::VolumeMonitor::get() won't working
    // Gio::init() will also call Glib::init() so we don't need to call it again.
    Gio::init();

    m_gmmVolumeMonitorPtr = VolumeMonitor::get();

    m_connections.append(m_gmmVolumeMonitorPtr->signal_mount_added().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_mountAdded)));
    m_connections.append(m_gmmVolumeMonitorPtr->signal_mount_removed().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_mountRemoved)));
    m_connections.append(m_gmmVolumeMonitorPtr->signal_mount_pre_unmount().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_mountPreRemoved)));
    m_connections.append(m_gmmVolumeMonitorPtr->signal_mount_changed().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_mountChanged)));

    m_connections.append(m_gmmVolumeMonitorPtr->signal_volume_added().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_volumeAdded)));
    m_connections.append(m_gmmVolumeMonitorPtr->signal_volume_removed().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_volumeRemoved)));
    m_connections.append(m_gmmVolumeMonitorPtr->signal_volume_changed().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_volumeChanged)));

    m_connections.append(m_gmmVolumeMonitorPtr->signal_drive_connected().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_driveConnected)));
    m_connections.append(m_gmmVolumeMonitorPtr->signal_drive_disconnected().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_driveDisconnected)));
    m_connections.append(m_gmmVolumeMonitorPtr->signal_drive_changed().connect(sigc::mem_fun(*this, &DGioVolumeManagerPrivate::slot_driveChanged)));
}

DGioVolumeManagerPrivate::~DGioVolumeManagerPrivate()
{
    for (auto & c : m_connections) {
        c.disconnect();
    }
    m_gmmVolumeMonitorPtr.release();
}

void DGioVolumeManagerPrivate::slot_mountAdded(const Glib::RefPtr<Mount> &gmmMount)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Mount> copy(gmmMount);
    QExplicitlySharedDataPointer<DGioMount> mount(new DGioMount(copy.release()));

    Q_EMIT q->mountAdded(mount);
}

void DGioVolumeManagerPrivate::slot_mountRemoved(const Glib::RefPtr<Mount> &gmmMount)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Mount> copy(gmmMount);
    QExplicitlySharedDataPointer<DGioMount> mount(new DGioMount(copy.release()));

    Q_EMIT q->mountRemoved(mount);
}

void DGioVolumeManagerPrivate::slot_mountPreRemoved(const Glib::RefPtr<Mount> &gmmMount)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Mount> copy(gmmMount);
    QExplicitlySharedDataPointer<DGioMount> mount(new DGioMount(copy.release()));

    Q_EMIT q->mountPreRemoved(mount);
}

void DGioVolumeManagerPrivate::slot_mountChanged(const Glib::RefPtr<Mount> &gmmMount)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Mount> copy(gmmMount);
    QExplicitlySharedDataPointer<DGioMount> mount(new DGioMount(copy.release()));

    Q_EMIT q->mountChanged(mount);
}

void DGioVolumeManagerPrivate::slot_volumeAdded(const Glib::RefPtr<Volume> &gmmVolume)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Volume> copy(gmmVolume);
    QExplicitlySharedDataPointer<DGioVolume> volume(new DGioVolume(copy.release()));

    Q_EMIT q->volumeAdded(volume);
}

void DGioVolumeManagerPrivate::slot_volumeRemoved(const Glib::RefPtr<Volume> &gmmVolume)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Volume> copy(gmmVolume);
    QExplicitlySharedDataPointer<DGioVolume> volume(new DGioVolume(copy.release()));

    Q_EMIT q->volumeRemoved(volume);
}

void DGioVolumeManagerPrivate::slot_volumeChanged(const Glib::RefPtr<Volume> &gmmVolume)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Volume> copy(gmmVolume);
    QExplicitlySharedDataPointer<DGioVolume> volume(new DGioVolume(copy.release()));

    Q_EMIT q->volumeChanged(volume);
}

void DGioVolumeManagerPrivate::slot_driveConnected(const Glib::RefPtr<Drive> &gmmDrive)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Drive> copy(gmmDrive);
    QExplicitlySharedDataPointer<DGioDrive> drive(new DGioDrive(copy.release()));

    Q_EMIT q->driveConnected(drive);
}

void DGioVolumeManagerPrivate::slot_driveDisconnected(const Glib::RefPtr<Drive> &gmmDrive)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Drive> copy(gmmDrive);
    QExplicitlySharedDataPointer<DGioDrive> drive(new DGioDrive(copy.release()));

    Q_EMIT q->driveDisconnected(drive);
}

void DGioVolumeManagerPrivate::slot_driveChanged(const Glib::RefPtr<Drive> &gmmDrive)
{
    Q_Q(DGioVolumeManager);

    Glib::RefPtr<Drive> copy(gmmDrive);
    QExplicitlySharedDataPointer<DGioDrive> drive(new DGioDrive(copy.release()));

    Q_EMIT q->driveChanged(drive);
}

// -------------------------------------------------------------

DGioVolumeManager::DGioVolumeManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DGioVolumeManagerPrivate(this))
{

}

DGioVolumeManager::~DGioVolumeManager()
{

}

const QList<QExplicitlySharedDataPointer<DGioMount> > DGioVolumeManager::getMounts()
{
    // ensure GIO got initialized
    Gio::init();

    QList<QExplicitlySharedDataPointer<DGioMount> > mounts;

    Glib::RefPtr<VolumeMonitor> vm = Gio::VolumeMonitor::get();

    Glib::ListHandle<Glib::RefPtr<Mount> > mnt = vm->get_mounts();
    for (Glib::RefPtr<Mount> oneMnt : mnt) {

        QExplicitlySharedDataPointer<DGioMount> mntPtr(new DGioMount(oneMnt.release()));
        mounts.append(mntPtr);
    }

    return mounts;
}

const QList<QExplicitlySharedDataPointer<DGioVolume> > DGioVolumeManager::getVolumes()
{
    // ensure GIO got initialized
    Gio::init();

    QList<QExplicitlySharedDataPointer<DGioVolume> > volumes;

    Glib::RefPtr<VolumeMonitor> vm = Gio::VolumeMonitor::get();

    Glib::ListHandle<Glib::RefPtr<Volume> > vol = vm->get_volumes();
    for (Glib::RefPtr<Volume> oneVol : vol) {

        QExplicitlySharedDataPointer<DGioVolume> volPtr(new DGioVolume(oneVol.release()));
        volumes.append(volPtr);
    }

    return volumes;
}

const QList<QExplicitlySharedDataPointer<DGioDrive> > DGioVolumeManager::getDrives()
{
    Gio::init();

    QList<QExplicitlySharedDataPointer<DGioDrive> > drives;

    Glib::RefPtr<VolumeMonitor> vm = Gio::VolumeMonitor::get();

    auto drvs = vm->get_connected_drives();
    for(auto oneDrive : drvs){
        QExplicitlySharedDataPointer<DGioDrive> drvPtr(new DGioDrive(oneDrive.release()));
        drives.push_back(drvPtr);
    }
    return drives;
}

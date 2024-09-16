// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DProtocolDeviceMonitor>
#include "dprotocoldevicemonitor_p.h"
#include "utils.h"

#include <gio/gunixmounts.h>

#include <QRegularExpression>

DMOUNT_USE_NAMESPACE

static constexpr char kSigVolumeAdded[] { "volume-added" };
static constexpr char kSigVolumeRemoved[] { "volume-removed" };
static constexpr char kSigMountAdded[] { "mount-added" };
static constexpr char kSigMountRemoved[] { "mount-removed" };

DProtocolDeviceMonitor::DProtocolDeviceMonitor(QObject *parent)
    : QObject { parent }, d_ptr { new DProtocolDeviceMonitorPrivate(this) }
{
}

DProtocolDeviceMonitor::~DProtocolDeviceMonitor()
{
}

void DProtocolDeviceMonitor::setWatchChanges(bool watchChanges)
{
    Q_D(DProtocolDeviceMonitor);
    watchChanges ? d->startMonitor() : d->stopMonitor();
}

bool DProtocolDeviceMonitor::watchChanges() const
{
    Q_D(const DProtocolDeviceMonitor);
    return d->isWatching;
}

QStringList DProtocolDeviceMonitor::devices() const
{
    Q_D(const DProtocolDeviceMonitor);
    return d->devUrls.values();
}

void DProtocolDeviceMonitorPrivate::initDevices()
{
    GList_autoptr vols = g_volume_monitor_get_volumes(monitor);
    g_list_foreach(vols, volumeFilter, this);

    GList_autoptr mounts = g_volume_monitor_get_mounts(monitor);
    g_list_foreach(mounts, mountFilter, this);
}

bool DProtocolDeviceMonitorPrivate::startMonitor()
{
    if (!monitor || isWatching)
        return false;

    ulong handler = 0;

    handler = g_signal_connect(monitor, kSigMountAdded, G_CALLBACK(DProtocolDeviceMonitorPrivate::onMountAdded), this);
    connections.insert(kSigMountAdded, handler);
    handler = g_signal_connect(monitor, kSigMountRemoved, G_CALLBACK(DProtocolDeviceMonitorPrivate::onMountRemoved), this);
    connections.insert(kSigMountRemoved, handler);
    handler = g_signal_connect(monitor, kSigVolumeAdded, G_CALLBACK(DProtocolDeviceMonitorPrivate::onVolumeAdded), this);
    connections.insert(kSigVolumeAdded, handler);
    handler = g_signal_connect(monitor, kSigVolumeRemoved, G_CALLBACK(DProtocolDeviceMonitorPrivate::onVolumeRemoved), this);
    connections.insert(kSigVolumeRemoved, handler);
    isWatching = true;
    return true;
}

bool DProtocolDeviceMonitorPrivate::stopMonitor()
{
    if (!monitor || !isWatching)
        return false;

    auto handlers = connections.values();
    for (auto handler : handlers)
        g_signal_handler_disconnect(monitor, handler);
    connections.clear();

    isWatching = false;
    return true;
}

void DProtocolDeviceMonitorPrivate::volumeFilter(void *vol, void *data)
{
    auto d = static_cast<DProtocolDeviceMonitorPrivate *>(data);
    Q_ASSERT(d);

    GVolume *gVol = static_cast<GVolume *>(vol);
    if (!gVol)
        return;

    GDrive_autoptr drv = g_volume_get_drive(gVol);
    if (drv)   // volume which have a drive is not protocol device, should be filtered.
        return;

    // devices can be founded before mount, such as MTP/GPHOTO...
    GFile_autoptr activeRoot = g_volume_get_activation_root(gVol);
    if (activeRoot) {
        g_autofree char *cUri = g_file_get_uri(activeRoot);
        d->devUrls.insert(cUri);
    } else {
        g_autofree char *dev = g_volume_get_identifier(gVol, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        qDebug() << "mount: active root for" << dev << "is null.";
    }
}

void DProtocolDeviceMonitorPrivate::mountFilter(void *mount, void *data)
{
    auto d = static_cast<DProtocolDeviceMonitorPrivate *>(data);
    Q_ASSERT(d);

    GMount *gMount = static_cast<GMount *>(mount);
    if (!gMount)
        return;

    // only orphan mounts are captured here, mounts which associated with volumes
    // are either blocks or have been captured in volume iterations.
    GVolume_autoptr vol = g_mount_get_volume(gMount);
    if (vol)
        return;

    GFile_autoptr root = g_mount_get_root(gMount);
    if (root) {
        g_autofree char *cPath = g_file_get_path(root);
        Q_ASSERT(cPath);   // path MUST not be NULL anytime for GMount
        if (isLocalSourceMount(cPath) || !isMountOfCurrentUser(cPath))
            return;

        g_autofree char *cUri = g_file_get_uri(root);
        d->devUrls.insert(cUri);
    } else {
        g_autofree char *cName = g_mount_get_name(gMount);
        qDebug() << "mount: root for" << cName << "is null.";
    }
}

bool DProtocolDeviceMonitorPrivate::hasAssociatedDrive(GMount *mount)
{
    if (mount) {
        GDrive_autoptr drv = g_mount_get_drive(mount);
        return drv;
    }
    return false;
}

bool DProtocolDeviceMonitorPrivate::hasAssociatedDrive(GVolume *vol)
{
    if (vol) {
        GDrive_autoptr drv = g_volume_get_drive(vol);
        return drv;
    }
    return false;
}

bool DProtocolDeviceMonitorPrivate::isLocalSourceMount(const QString &mpt)
{
    Q_ASSERT(!mpt.isEmpty());

    GUnixMountEntry_autoptr entry = g_unix_mount_for(mpt.toStdString().c_str(), nullptr);
    if (entry) {
        QString source = g_unix_mount_get_device_path(entry);
        return source.startsWith("/dev/");
    }
    return false;
}

bool DProtocolDeviceMonitorPrivate::isMountOfCurrentUser(const QString &mpt)
{
    Q_ASSERT(!mpt.isEmpty());

    static QRegularExpression regx(Utils::kRegxDaemonMountPath);
    auto match = regx.match(mpt);
    if (match.hasMatch()) {
        auto user = match.captured(1);
        return user == Utils::workingUser();
    }
    return true;
}

bool DProtocolDeviceMonitorPrivate::isOrphanMount(GMount *mount)
{
    Q_ASSERT(mount);

    GFile_autoptr root = g_mount_get_root(mount);
    if (!root)
        return false;

    g_autofree char *cUri = g_file_get_uri(root);
    QString uri(cUri);
    static QRegularExpression regx(R"(^smb|^ftp|^sftp|^dav|^webdav)");
    if (uri.contains(regx))
        return true;

    GVolumeMonitor_autoptr monitor = g_volume_monitor_get();
    GList_autoptr vols = g_volume_monitor_get_volumes(monitor);
    while (vols) {
        auto vol = reinterpret_cast<GVolume *>(vols->data);
        GFile_autoptr activeRoot = g_volume_get_activation_root(vol);
        if (activeRoot) {
            g_autofree char *cActiveUri = g_file_get_uri(activeRoot);
            if (g_strcmp0(cUri, cActiveUri) == 0)
                return false;
        }
        vols = vols->next;
    }
    return true;
}

void DProtocolDeviceMonitorPrivate::onMountAdded(GVolumeMonitor *, GMount *mount, gpointer data)
{
    if (hasAssociatedDrive(mount))
        return;

    const auto &&info = Utils::mountInfoOf(mount);
    auto &&mpt = info.second;
    if (isLocalSourceMount(mpt) || !isMountOfCurrentUser(mpt))
        return;

    auto &&uri = info.first;
    if (!uri.isEmpty()) {
        auto d = static_cast<DProtocolDeviceMonitorPrivate *>(data);
        Q_ASSERT(d);

        d->devUrls.insert(uri);
        Q_EMIT d->q_func()->mountAdded(uri, mpt);
    }
}

void DProtocolDeviceMonitorPrivate::onMountRemoved(GVolumeMonitor *, GMount *mount, gpointer data)
{
    if (hasAssociatedDrive(mount))
        return;

    auto d = static_cast<DProtocolDeviceMonitorPrivate *>(data);
    Q_ASSERT(d);

    const auto &&info = Utils::mountInfoOf(mount);
    auto &&uri = info.first;
    if (isOrphanMount(mount))
        d->devUrls.remove(uri);
    Q_EMIT d->q_func()->mountRemoved(uri);
}

void DProtocolDeviceMonitorPrivate::onVolumeAdded(GVolumeMonitor *, GVolume *volume, gpointer data)
{
    if (hasAssociatedDrive(volume))
        return;

    auto d = static_cast<DProtocolDeviceMonitorPrivate *>(data);
    Q_ASSERT(d);

    GFile_autoptr activeRoot = g_volume_get_activation_root(volume);
    if (activeRoot) {
        g_autofree char *cUri = g_file_get_uri(activeRoot);
        d->devUrls.insert(cUri);
        Q_EMIT d->q_func()->deviceAdded(cUri);
    } else {
        qDebug() << "mount: cannot obtain the activation root of volume " << __FUNCTION__;
    }
}

void DProtocolDeviceMonitorPrivate::onVolumeRemoved(GVolumeMonitor *, GVolume *volume, gpointer data)
{
    if (hasAssociatedDrive(volume))
        return;

    auto d = static_cast<DProtocolDeviceMonitorPrivate *>(data);
    Q_ASSERT(d);

    GFile_autoptr activeRoot = g_volume_get_activation_root(volume);
    if (activeRoot) {
        g_autofree char *cUri = g_file_get_uri(activeRoot);
        d->devUrls.remove(cUri);
        Q_EMIT d->q_func()->deviceRemoved(cUri);
    } else {
        qDebug() << "mount: cannot obtain the activation root of volume " << __FUNCTION__;
    }
}

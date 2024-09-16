// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgiomount.h"
#include "dgiovolume.h"
#include "dgiofile.h"

#include <glibmm/refptr.h>

#include <giomm/init.h>
#include <giomm/file.h>
#include <giomm/volume.h>
#include <giomm/themedicon.h>

#include <QDebug>

// private
#include "dgiohelper.h"

using namespace Gio;

class DGioMountPrivate
{
public:
    DGioMountPrivate(DGioMount *qq, Mount *gmmMountPtr);

    Glib::RefPtr<Mount> getGmmMountInstance() const;

    QString name() const;
    QString uuid() const;
    bool canUnmount() const;
    bool canEject() const;

private:
    Glib::RefPtr<Mount> m_gmmMountPtr;

    DGioMount *q_ptr;

    Q_DECLARE_PUBLIC(DGioMount)
};

DGioMountPrivate::DGioMountPrivate(DGioMount *qq, Mount *gmmMountPtr)
    : m_gmmMountPtr(gmmMountPtr)
    , q_ptr(qq)
{

}

Glib::RefPtr<Mount> DGioMountPrivate::getGmmMountInstance() const
{
    return m_gmmMountPtr;
}

QString DGioMountPrivate::name() const
{
    return QString::fromStdString(m_gmmMountPtr->get_name());
}

QString DGioMountPrivate::uuid() const
{
    return QString::fromStdString(m_gmmMountPtr->get_uuid());
}

// -------------------------------------------------------------

DGioMount::DGioMount(Mount *gmmMountPtr, QObject *parent)
    : QObject(parent)
    , d_ptr(new DGioMountPrivate(this, gmmMountPtr))
{
    // gmountPtr must be valid;
    Q_CHECK_PTR(gmmMountPtr);
}

DGioMount::~DGioMount()
{

}

/*!
 * \brief Create a DGioMount instance by a given \a path
 *
 * \return the created DGioMount instance or nullptr if failed.
 */
DGioMount *DGioMount::createFromPath(QString path, QObject *parent)
{
    // ensure GIO got initialized
    Gio::init();

    Glib::RefPtr<File> gmmFile = File::create_for_path(path.toStdString());
    try {
        Glib::RefPtr<Mount> gmmMount = gmmFile->find_enclosing_mount();
        if (gmmMount) {
            return new DGioMount(gmmMount.release(), parent);
        }
    } catch (const Glib::Error &error) {
        qDebug() << QString::fromStdString(error.what().raw());
    }

    return nullptr;
}

QString DGioMount::name() const
{
    Q_D(const DGioMount);

    return d->name();
}

QString DGioMount::uuid() const
{
    Q_D(const DGioMount);

    return d->uuid();
}

QString DGioMount::mountClass() const
{
    Q_D(const DGioMount);

    return QString(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(d->m_gmmMountPtr->gobj())));
}

/*!
 * \brief Determines if mount is shadowed.
 *
 * A mount is said to be shadowed if there exists one or more user visible objects (currently Mount objects)
 * with a root that is inside the root of mount.
 *
 * One application of shadow mounts is when exposing a single file system that is used to address several
 * logical volumes. In this situation, a Gio::VolumeMonitor implementation would create two Gio::Volume
 * objects, in gio-qt, DGioVolumeManager will create two DGioVolume objects.
 *
 * For example, one for the camera functionality of the device and one for a SD card reader on the device) with
 * activation URIs gphoto2://[usb:001,002]/store1/ and gphoto2://[usb:001,002]/store2/. When the underlying
 * mount (with root gphoto2://[usb:001,002]/) is mounted, said VolumeMonitor implementation would create two
 * Mount objects (each with their root matching the corresponding volume activation root) that would shadow
 * the original mount.
 *
 * \return true if mount is shadowed
 */
bool DGioMount::isShadowed() const
{
    Q_D(const DGioMount);

    return d->getGmmMountInstance()->is_shadowed();
}

bool DGioMount::canUnmount() const
{
    Q_D(const DGioMount);

    return d->getGmmMountInstance()->can_unmount();
}

bool DGioMount::canEject() const
{
    Q_D(const DGioMount);

    return d->getGmmMountInstance()->can_eject();
}

QString DGioMount::sortKey() const
{
    Q_D(const DGioMount);

    return QString::fromStdString(d->getGmmMountInstance()->get_sort_key());
}

QStringList DGioMount::themedIconNames() const
{
    Q_D(const DGioMount);

    Glib::RefPtr<const Icon> icon = d->getGmmMountInstance()->get_icon();

    return DGioPrivate::getThemedIconNames(icon);
}

QStringList DGioMount::themedSymbolicIconNames() const
{
    Q_D(const DGioMount);

    Glib::RefPtr<const Icon> icon = d->getGmmMountInstance()->get_symbolic_icon();

    return DGioPrivate::getThemedIconNames(icon);
}

void DGioMount::unmount(bool forceUnmount)
{
    Q_D(const DGioMount);

    return d->getGmmMountInstance()->unmount(forceUnmount ? MOUNT_UNMOUNT_FORCE : MOUNT_UNMOUNT_NONE);
}

void DGioMount::eject(bool forceEject)
{
    Q_D(const DGioMount);

    return d->getGmmMountInstance()->eject(forceEject ? MOUNT_UNMOUNT_FORCE : MOUNT_UNMOUNT_NONE);
}

QExplicitlySharedDataPointer<DGioFile> DGioMount::getRootFile()
{
    Q_D(const DGioMount);

    Glib::RefPtr<File> file = d->getGmmMountInstance()->get_root();
    QExplicitlySharedDataPointer<DGioFile> filePtr(new DGioFile(file.release()));

    return filePtr;
}

QExplicitlySharedDataPointer<DGioFile> DGioMount::getDefaultLocationFile()
{
    Q_D(const DGioMount);

    Glib::RefPtr<File> file = d->getGmmMountInstance()->get_default_location();
    QExplicitlySharedDataPointer<DGioFile> filePtr(new DGioFile(file.release()));

    return filePtr;
}

QExplicitlySharedDataPointer<DGioVolume> DGioMount::getVolume()
{
    Q_D(const DGioMount);

    Glib::RefPtr<Volume> vol = d->getGmmMountInstance()->get_volume();
    if (vol) {
        QExplicitlySharedDataPointer<DGioVolume> volPtr(new DGioVolume(vol.release()));
        return volPtr;
    }

    return QExplicitlySharedDataPointer<DGioVolume>(nullptr);
}

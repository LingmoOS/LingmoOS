// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgiodrive.h"

#include <glibmm/refptr.h>
#include <giomm/drive.h>

using namespace Gio;

class DGioDrivePrivate{
public:
    DGioDrivePrivate(DGioDrive *qq, Drive *gmmDrivePtr);

    Glib::RefPtr<Drive> getGmmDriveInstence() const;

    QString name() const;

private:
    Glib::RefPtr<Drive> m_gmmDrivePtr;

    DGioDrive *q_ptr;
    Q_DECLARE_PUBLIC(DGioDrive)
};

DGioDrivePrivate::DGioDrivePrivate(DGioDrive *qq, Drive *gmmDrivePtr)
    : m_gmmDrivePtr(gmmDrivePtr)
    , q_ptr(qq)
{

}

Glib::RefPtr<Drive> DGioDrivePrivate::getGmmDriveInstence() const
{
    return m_gmmDrivePtr;
}

QString DGioDrivePrivate::name() const
{
    return QString::fromStdString(m_gmmDrivePtr->get_name());
}

// -------------------------------------------------------------

DGioDrive::DGioDrive(Gio::Drive *gmmDrivePtr, QObject *parent)
    : QObject(parent)
    , d_ptr(new DGioDrivePrivate(this, gmmDrivePtr))
{
    Q_CHECK_PTR(gmmDrivePtr);
}

DGioDrive::~DGioDrive()
{

}

/*!
 * \brief Gets the name of drive.
 *
 * Wrapper of Gio::Drive::get_name()
 */
QString DGioDrive::name() const
{
    Q_D(const DGioDrive);

    return d->name();
}

/*!
 * \brief Gets the identifier of the given kind for drive.
 *
 * Wrapper of Gio::Drive::get_identifier()
 *
 * The only identifier currently available is DGIODRIVE_IDENTIFIER_KIND_UNIX_DEVICE.
 *
 * \param kind the kind of identifier to return
 *
 * \return A string containing the requested identfier, or empty string if the drive doesn't have this kind of identifier.
 */
QString DGioDrive::identifier(const QString &kind) const
{
    Q_D(const DGioDrive);

    return QString::fromStdString(d->getGmmDriveInstence()->get_identifier(kind.toStdString()));
}

bool DGioDrive::hasVolumes() const
{
    Q_D(const DGioDrive);

    return d->getGmmDriveInstence()->has_volumes();
}

bool DGioDrive::canStart() const
{
    Q_D(const DGioDrive);

    return d->getGmmDriveInstence()->can_start();
}

bool DGioDrive::canStop() const
{
    Q_D(const DGioDrive);

    return d->getGmmDriveInstence()->can_stop();
}

bool DGioDrive::canEject() const
{
    Q_D(const DGioDrive);

    return d->getGmmDriveInstence()->can_eject();
}

/*!
 * \brief Checks if the drive supports removable media.
 *
 * Wrapper of Gio::Drive::is_media_removable()
 *
 * \return true if drive supports removable media, false otherwise.
 */
bool DGioDrive::isMediaRemovable() const
{
    Q_D(const DGioDrive);

    return d->getGmmDriveInstence()->is_media_removable();
}

/*!
 * \brief Checks if the drive and/or its media is considered removable by the user.
 *
 * Wrapper of Gio::Drive::is_removable()
 *
 * \return true if drive and/or its media is considered removable, false otherwise.
 *
 * \sa isMediaRemovable()
 */
bool DGioDrive::isRemovable() const
{
    Q_D(const DGioDrive);

    return d->getGmmDriveInstence()->is_removable();
}

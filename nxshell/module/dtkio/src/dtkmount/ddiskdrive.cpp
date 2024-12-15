// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddiskdrive_p.h"
#include "udisks2_interface.h"

DMOUNT_BEGIN_NAMESPACE

DDiskDrive::~DDiskDrive()
{
}

QString DDiskDrive::path() const
{
    Q_D(const DDiskDrive);

    return d->dbus->path();
}

bool DDiskDrive::canPowerOff() const
{
    Q_D(const DDiskDrive);

    return d->dbus->canPowerOff();
}

QVariantMap DDiskDrive::configuration() const
{
    Q_D(const DDiskDrive);

    return d->dbus->configuration();
}

QString DDiskDrive::connectionBus() const
{
    Q_D(const DDiskDrive);

    return d->dbus->connectionBus();
}

bool DDiskDrive::ejectable() const
{
    Q_D(const DDiskDrive);

    return d->dbus->ejectable();
}

QString DDiskDrive::id() const
{
    Q_D(const DDiskDrive);

    return d->dbus->id();
}

QString DDiskDrive::media() const
{
    Q_D(const DDiskDrive);

    return d->dbus->media();
}

bool DDiskDrive::mediaAvailable() const
{
    Q_D(const DDiskDrive);

    return d->dbus->mediaAvailable();
}

bool DDiskDrive::mediaChangeDetected() const
{
    Q_D(const DDiskDrive);

    return d->dbus->mediaChangeDetected();
}

QStringList DDiskDrive::mediaCompatibility() const
{
    Q_D(const DDiskDrive);

    return d->dbus->mediaCompatibility();
}

bool DDiskDrive::mediaRemovable() const
{
    Q_D(const DDiskDrive);

    return d->dbus->mediaRemovable();
}

QString DDiskDrive::model() const
{
    Q_D(const DDiskDrive);

    return d->dbus->model();
}

bool DDiskDrive::optical() const
{
    Q_D(const DDiskDrive);

    return d->dbus->optical();
}

bool DDiskDrive::opticalBlank() const
{
    Q_D(const DDiskDrive);

    return d->dbus->opticalBlank();
}

uint DDiskDrive::opticalNumAudioTracks() const
{
    Q_D(const DDiskDrive);

    return d->dbus->opticalNumAudioTracks();
}

uint DDiskDrive::opticalNumDataTracks() const
{
    Q_D(const DDiskDrive);

    return d->dbus->opticalNumDataTracks();
}

uint DDiskDrive::opticalNumSessions() const
{
    Q_D(const DDiskDrive);

    return d->dbus->opticalNumSessions();
}

uint DDiskDrive::opticalNumTracks() const
{
    Q_D(const DDiskDrive);

    return d->dbus->opticalNumTracks();
}

bool DDiskDrive::removable() const
{
    Q_D(const DDiskDrive);

    return d->dbus->removable();
}

QString DDiskDrive::revision() const
{
    Q_D(const DDiskDrive);

    return d->dbus->revision();
}

int DDiskDrive::rotationRate() const
{
    Q_D(const DDiskDrive);

    return d->dbus->rotationRate();
}

QString DDiskDrive::seat() const
{
    Q_D(const DDiskDrive);

    return d->dbus->seat();
}

QString DDiskDrive::serial() const
{
    Q_D(const DDiskDrive);

    return d->dbus->serial();
}

QString DDiskDrive::siblingId() const
{
    Q_D(const DDiskDrive);

    return d->dbus->siblingId();
}

quint64 DDiskDrive::size() const
{
    Q_D(const DDiskDrive);

    return d->dbus->size();
}

QString DDiskDrive::sortKey() const
{
    Q_D(const DDiskDrive);

    return d->dbus->sortKey();
}

quint64 DDiskDrive::timeDetected() const
{
    Q_D(const DDiskDrive);

    return d->dbus->timeDetected();
}

quint64 DDiskDrive::timeMediaDetected() const
{
    Q_D(const DDiskDrive);

    return d->dbus->timeMediaDetected();
}

QString DDiskDrive::vendor() const
{
    Q_D(const DDiskDrive);

    return d->dbus->vendor();
}

QString DDiskDrive::WWN() const
{
    Q_D(const DDiskDrive);

    return d->dbus->wWN();
}

void DDiskDrive::eject(const QVariantMap &options)
{
    Q_D(DDiskDrive);

    auto r = d_ptr->dbus->Eject(options);
    r.waitForFinished();
    d->err = r.error();
}

void DDiskDrive::powerOff(const QVariantMap &options)
{
    Q_D(DDiskDrive);

    auto r = d_ptr->dbus->PowerOff(options);
    r.waitForFinished();
    d->err = r.error();
}

void DDiskDrive::setConfiguration(const QVariantMap &value, const QVariantMap &options)
{
    Q_D(DDiskDrive);

    auto r = d_ptr->dbus->SetConfiguration(value, options);
    r.waitForFinished();
    d->err = r.error();
}

DDiskDrive::DDiskDrive(const QString &path, QObject *parent)
    : QObject(parent), d_ptr(new DDiskDrivePrivate())
{
    d_ptr->dbus = new OrgFreedesktopUDisks2DriveInterface(kUDisks2Service, path, QDBusConnection::systemBus(), this);
}

DMOUNT_END_NAMESPACE

/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSOPTICALDRIVE_H
#define UDISKSOPTICALDRIVE_H

#include "udisksstoragedrive.h"
#include <solid/devices/ifaces/opticaldrive.h>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class OpticalDrive : public StorageDrive, virtual public Solid::Ifaces::OpticalDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDrive)

public:
    OpticalDrive(Device *device);
    ~OpticalDrive() override;

Q_SIGNALS:
    void ejectPressed(const QString &udi) override;
    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi) override;
    void ejectRequested(const QString &udi);

public:
    bool eject() override;
    QList<int> writeSpeeds() const override;
    int writeSpeed() const override;
    int readSpeed() const override;
    Solid::OpticalDrive::MediumTypes supportedMedia() const override;

private Q_SLOTS:
    void slotDBusReply(const QDBusMessage &reply);
    void slotDBusError(const QDBusError &error);

    void slotEjectRequested();
    void slotEjectDone(int error, const QString &errorString);

    void slotChanged();

private:
    void initReadWriteSpeeds() const;

    bool m_ejectInProgress;

    // read/write speeds
    mutable int m_readSpeed;
    mutable int m_writeSpeed;
    mutable QList<int> m_writeSpeeds;
    mutable bool m_speedsInit;
};

}
}
}

#endif // UDISKSOPTICALDRIVE_H

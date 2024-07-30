/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksopticaldrive.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QDebug>
#include <QFile>

#include "dbus/manager.h"
#include "udisks2.h"
#include "udisks_debug.h"
#include "udisksdevice.h"

using namespace Solid::Backends::UDisks2;

OpticalDrive::OpticalDrive(Device *device)
    : StorageDrive(device)
    , m_ejectInProgress(false)
    , m_readSpeed(0)
    , m_writeSpeed(0)
    , m_speedsInit(false)
{
    m_device->registerAction("eject", this, SLOT(slotEjectRequested()), SLOT(slotEjectDone(int, QString)));

    connect(m_device, SIGNAL(changed()), this, SLOT(slotChanged()));
}

OpticalDrive::~OpticalDrive()
{
}

bool OpticalDrive::eject()
{
    if (m_ejectInProgress) {
        return false;
    }
    m_ejectInProgress = true;
    m_device->broadcastActionRequested("eject");

    const QString path = m_device->udi();
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus, "Solid::Udisks2::OpticalDrive::" + path);

    // if the device is mounted, unmount first
    QString blockPath;
    org::freedesktop::DBus::ObjectManager manager(UD2_DBUS_SERVICE, UD2_DBUS_PATH, c);
    QDBusPendingReply<DBUSManagerStruct> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (!reply.isError()) { // enum devices
        const auto objPathMap = reply.value();
        for (auto it = objPathMap.cbegin(); it != objPathMap.cend(); ++it) {
            const QString udi = it.key().path();

            // qDebug() << "Inspecting" << udi;

            if (udi == UD2_DBUS_PATH_MANAGER || udi == UD2_UDI_DISKS_PREFIX || udi.startsWith(UD2_DBUS_PATH_JOBS)) {
                continue;
            }

            Device device(udi);
            if (device.drivePath() == path && device.isMounted()) {
                // qDebug() << "Got mounted block device:" << udi;
                blockPath = udi;
                break;
            }
        }
    } else { // show error
        qCWarning(UDISKS2) << "Failed enumerating UDisks2 objects:" << reply.error().name() << "\n" << reply.error().message();
    }

    if (!blockPath.isEmpty()) {
        // qDebug() << "Calling unmount on" << blockPath;
        QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, blockPath, UD2_DBUS_INTERFACE_FILESYSTEM, "Unmount");
        msg << QVariantMap(); // options, unused now
        c.call(msg, QDBus::BlockWithGui);
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, path, UD2_DBUS_INTERFACE_DRIVE, "Eject");
    msg << QVariantMap();
    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

void OpticalDrive::slotDBusReply(const QDBusMessage & /*reply*/)
{
    m_ejectInProgress = false;
    m_device->broadcastActionDone("eject");
}

void OpticalDrive::slotDBusError(const QDBusError &error)
{
    m_ejectInProgress = false;
    m_device->broadcastActionDone("eject", //
                                  m_device->errorToSolidError(error.name()),
                                  m_device->errorToString(error.name()) + ": " + error.message());
}

void OpticalDrive::slotEjectRequested()
{
    m_ejectInProgress = true;
    Q_EMIT ejectRequested(m_device->udi());
}

void OpticalDrive::slotEjectDone(int error, const QString &errorString)
{
    m_ejectInProgress = false;
    Q_EMIT ejectDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

void OpticalDrive::initReadWriteSpeeds() const
{
#if 0
    int read_speed, write_speed;
    char *write_speeds = 0;
    QByteArray device_file = QFile::encodeName(m_device->property("Device").toString());

    //qDebug("Doing open (\"%s\", O_RDONLY | O_NONBLOCK)", device_file.constData());
    int fd = open(device_file, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        qWarning("Cannot open %s: %s", device_file.constData(), strerror(errno));
        return;
    }

    if (get_read_write_speed(fd, &read_speed, &write_speed, &write_speeds) >= 0) {
        m_readSpeed = read_speed;
        m_writeSpeed = write_speed;

        QStringList list = QString::fromLatin1(write_speeds).split(',', Qt::SkipEmptyParts);
        Q_FOREACH (const QString &speed, list) {
            m_writeSpeeds.append(speed.toInt());
        }

        free(write_speeds);

        m_speedsInit = true;
    }

    close(fd);
#endif
}

QList<int> OpticalDrive::writeSpeeds() const
{
    if (!m_speedsInit) {
        initReadWriteSpeeds();
    }
    // qDebug() << "solid write speeds:" << m_writeSpeeds;
    return m_writeSpeeds;
}

int OpticalDrive::writeSpeed() const
{
    if (!m_speedsInit) {
        initReadWriteSpeeds();
    }
    return m_writeSpeed;
}

int OpticalDrive::readSpeed() const
{
    if (!m_speedsInit) {
        initReadWriteSpeeds();
    }
    return m_readSpeed;
}

Solid::OpticalDrive::MediumTypes OpticalDrive::supportedMedia() const
{
    const QStringList mediaTypes = m_device->prop("MediaCompatibility").toStringList();
    Solid::OpticalDrive::MediumTypes supported;

    QMap<QString, Solid::OpticalDrive::MediumType> map = {
        {QStringLiteral("optical_cd_r"), Solid::OpticalDrive::Cdr},
        {QStringLiteral("optical_cd_rw"), Solid::OpticalDrive::Cdrw},
        {QStringLiteral("optical_dvd"), Solid::OpticalDrive::Dvd},
        {QStringLiteral("optical_dvd_r"), Solid::OpticalDrive::Dvdr},
        {QStringLiteral("optical_dvd_rw"), Solid::OpticalDrive::Dvdrw},
        {QStringLiteral("optical_dvd_ram"), Solid::OpticalDrive::Dvdram},
        {QStringLiteral("optical_dvd_plus_r"), Solid::OpticalDrive::Dvdplusr},
        {QStringLiteral("optical_dvd_plus_rw"), Solid::OpticalDrive::Dvdplusrw},
        {QStringLiteral("optical_dvd_plus_r_dl"), Solid::OpticalDrive::Dvdplusdl},
        {QStringLiteral("optical_dvd_plus_rw_dl"), Solid::OpticalDrive::Dvdplusdlrw},
        {QStringLiteral("optical_bd"), Solid::OpticalDrive::Bd},
        {QStringLiteral("optical_bd_r"), Solid::OpticalDrive::Bdr},
        {QStringLiteral("optical_bd_re"), Solid::OpticalDrive::Bdre},
        {QStringLiteral("optical_hddvd"), Solid::OpticalDrive::HdDvd},
        {QStringLiteral("optical_hddvd_r"), Solid::OpticalDrive::HdDvdr},
        {QStringLiteral("optical_hddvd_rw"), Solid::OpticalDrive::HdDvdrw},
    };

    // TODO add these to Solid
    // map[Solid::OpticalDrive::Mo] ="optical_mo";
    // map[Solid::OpticalDrive::Mr] ="optical_mrw";
    // map[Solid::OpticalDrive::Mrw] ="optical_mrw_w";

    for (const QString &media : mediaTypes) {
        supported |= map.value(media, Solid::OpticalDrive::UnknownMediumType);
    }

    return supported;
}

void OpticalDrive::slotChanged()
{
    m_speedsInit = false; // reset the read/write speeds, changes eg. with an inserted media
}

#include "moc_udisksopticaldrive.cpp"

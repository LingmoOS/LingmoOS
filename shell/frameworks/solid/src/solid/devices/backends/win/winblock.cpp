/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "winblock.h"

#include <QDebug>
#include <QSettings>

using namespace Solid::Backends::Win;

#include <ntddcdrm.h>
#include <ntddmmc.h>

QMap<QString, QString> WinBlock::m_driveLetters = QMap<QString, QString>();
QMap<QString, QSet<QString>> WinBlock::m_driveUDIS = QMap<QString, QSet<QString>>();
QMap<QString, QString> WinBlock::m_virtualDrives = QMap<QString, QString>();

WinBlock::WinBlock(WinDevice *device)
    : WinInterface(device)
    , m_major(-1)
    , m_minor(-1)
{
    if (m_device->type() == Solid::DeviceInterface::StorageVolume) {
        STORAGE_DEVICE_NUMBER info =
            WinDeviceManager::getDeviceInfo<STORAGE_DEVICE_NUMBER>(driveLetterFromUdi(m_device->udi()), IOCTL_STORAGE_GET_DEVICE_NUMBER);
        m_major = info.DeviceNumber;
        m_minor = info.PartitionNumber;
    } else if (m_device->type() == Solid::DeviceInterface::StorageDrive //
               || m_device->type() == Solid::DeviceInterface::OpticalDrive //
               || m_device->type() == Solid::DeviceInterface::OpticalDisc) {
        m_major = m_device->udi().mid(m_device->udi().length() - 1).toInt();
    } else {
        qFatal("Not implemented device type %i", m_device->type());
    }
}

WinBlock::~WinBlock()
{
}

int WinBlock::deviceMajor() const
{
    Q_ASSERT(m_major != -1);
    return m_major;
}

int WinBlock::deviceMinor() const
{
    return m_minor;
}

QString WinBlock::device() const
{
    return driveLetterFromUdi(m_device->udi());
}

QStringList WinBlock::drivesFromMask(const DWORD unitmask)
{
    QStringList result;
    DWORD localUnitmask(unitmask);
    for (int i = 0; i <= 25; ++i) {
        if (0x01 == (localUnitmask & 0x1)) {
            result << QString("%1:").arg((char)(i + 'A'));
        }
        localUnitmask >>= 1;
    }
    return result;
}

QSet<QString> WinBlock::getUdis()
{
    return updateUdiFromBitMask(GetLogicalDrives());
}

QString WinBlock::driveLetterFromUdi(const QString &udi)
{
    if (!m_driveLetters.contains(udi)) {
        qWarning() << udi << "is not connected to a drive";
    }
    return m_driveLetters[udi];
}

QString WinBlock::udiFromDriveLetter(const QString &drive)
{
    QString out;
    for (QMap<QString, QString>::const_iterator it = m_driveLetters.cbegin(); it != m_driveLetters.cend(); ++it) {
        if (it.value() == drive) {
            out = it.key();
            break;
        }
    }
    return out;
}

QString WinBlock::resolveVirtualDrive(const QString &drive)
{
    return m_virtualDrives[drive];
}

QSet<QString> WinBlock::updateUdiFromBitMask(const DWORD unitmask)
{
    const QStringList drives = drivesFromMask(unitmask);
    QSet<QString> list;
    wchar_t driveWCHAR[MAX_PATH];
    wchar_t bufferOut[MAX_PATH];
    QString dosPath;
    for (const QString &drive : drives) {
        QSet<QString> udis;
        driveWCHAR[drive.toWCharArray(driveWCHAR)] = 0;
        if (GetDriveType(driveWCHAR) == DRIVE_REMOTE) { // network drive
            QSettings settings(QLatin1String("HKEY_CURRENT_USER\\Network\\") + drive.at(0), QSettings::NativeFormat);
            QString path = settings.value("RemotePath").toString();
            if (!path.isEmpty()) {
                QString key = QLatin1String("/org/kde/solid/win/volume.virtual/") + drive.at(0);
                m_virtualDrives[key] = path;
                udis << key;
            }

        } else {
            QueryDosDeviceW(driveWCHAR, bufferOut, MAX_PATH);
            dosPath = QString::fromWCharArray(bufferOut);
            if (dosPath.startsWith("\\??\\")) { // subst junction
                dosPath = dosPath.mid(4);
                QString key = QLatin1String("/org/kde/solid/win/volume.virtual/") + drive.at(0);
                m_virtualDrives[key] = dosPath;
                udis << key;
            } else {
                STORAGE_DEVICE_NUMBER info = WinDeviceManager::getDeviceInfo<STORAGE_DEVICE_NUMBER>(drive, IOCTL_STORAGE_GET_DEVICE_NUMBER);

                switch (info.DeviceType) {
                case FILE_DEVICE_DISK: {
                    udis << QString("/org/kde/solid/win/volume/disk#%1,partition#%2").arg(info.DeviceNumber).arg(info.PartitionNumber);
                    udis << QString("/org/kde/solid/win/storage/disk#%1").arg(info.DeviceNumber);
                    break;
                }
                case FILE_DEVICE_CD_ROM:
                case FILE_DEVICE_DVD: {
                    udis << QString("/org/kde/solid/win/storage.cdrom/disk#%1").arg(info.DeviceNumber);
                    DISK_GEOMETRY_EX out = WinDeviceManager::getDeviceInfo<DISK_GEOMETRY_EX>(drive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX);
                    if (out.DiskSize.QuadPart != 0) {
                        udis << QString("/org/kde/solid/win/volume.cdrom/disk#%1").arg(info.DeviceNumber);
                    }
                    break;
                }
                default:
                    qDebug() << "unknown device" << drive << info.DeviceType << info.DeviceNumber << info.PartitionNumber;
                }
            }
        }
        m_driveUDIS[drive] = udis;
        for (const QString &str : std::as_const(udis)) {
            m_driveLetters[str] = drive;
        }
        list += udis;
    }
    return list;
}

QSet<QString> WinBlock::getFromBitMask(const DWORD unitmask)
{
    QSet<QString> list;
    const QStringList drives = drivesFromMask(unitmask);
    for (const QString &drive : drives) {
        if (m_driveUDIS.contains(drive)) {
            list += m_driveUDIS[drive];
        } else {
            // we have to update the cache
            return updateUdiFromBitMask(unitmask);
        }
    }
    return list;
}

#include "moc_winblock.cpp"

/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINBLOCK_H
#define WINBLOCK_H

#include "wininterface.h"
#include <solid/devices/ifaces/block.h>

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinBlock : public WinInterface, virtual public Solid::Ifaces::Block
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Block)
public:
    WinBlock(WinDevice *device);
    ~WinBlock();

    virtual int deviceMajor() const;

    virtual int deviceMinor() const;

    virtual QString device() const;

    static QSet<QString> getUdis();

    static QString driveLetterFromUdi(const QString &udi);
    static QString udiFromDriveLetter(const QString &drive);
    static QString resolveVirtualDrive(const QString &drive);
    static QSet<QString> updateUdiFromBitMask(const DWORD unitmask);
    static QSet<QString> getFromBitMask(const DWORD unitmask);

private:
    static QMap<QString, QString> m_driveLetters;
    static QMap<QString, QSet<QString>> m_driveUDIS;
    static QMap<QString, QString> m_virtualDrives;

    int m_major;
    int m_minor;

    static QStringList drivesFromMask(const DWORD unitmask);
};

}
}
}

#endif // WINBLOCK_H

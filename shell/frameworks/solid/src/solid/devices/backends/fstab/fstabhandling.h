/*
    SPDX-FileCopyrightText: 2006-2010 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FSTAB_FSTABHANDLING_H
#define SOLID_BACKENDS_FSTAB_FSTABHANDLING_H

#include <QMultiHash>
#include <QString>

#include <functional>

class QProcess;
class QObject;

namespace Solid
{
namespace Backends
{
namespace Fstab
{
class FstabHandling
{
public:
    FstabHandling();

    static QStringList deviceList();
    static QStringList currentMountPoints(const QString &device);
    static QStringList mountPoints(const QString &device);
    static QStringList options(const QString &device);
    static QString fstype(const QString &device);
    static bool callSystemCommand(const QString &commandName, const QStringList &args, const QObject *recvr, std::function<void(QProcess *)> callback);
    static void flushMtabCache();
    static void flushFstabCache();

private:
    static void _k_updateMtabMountPointsCache();
    static void _k_updateFstabMountPointsCache();

    typedef QMultiHash<QString, QString> QStringMultiHash;

    QStringMultiHash m_mtabCache;
    QStringMultiHash m_fstabCache;
    QStringMultiHash m_fstabOptionsCache;
    QHash<QString, QString> m_fstabFstypeCache;
    bool m_fstabCacheValid;
    bool m_mtabCacheValid;
};

}
}
}

#endif

/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "types.h"

#include <QFileInfo>

namespace PlasmaVault
{
static QString normalizePath(const QString &path)
{
    QFileInfo fileInfo(path);

    auto result = fileInfo.canonicalFilePath();

    if (result.isEmpty()) {
        result = path;
    }

    if (result.endsWith('/')) {
        result.chop(1);
    }

    return result;
}

Device::Device(const QString &device)
    : m_device(device)
{
}

QString Device::data() const
{
    // Done here because canonicalFilePath relies on file existence
    return normalizePath(m_device);
}

MountPoint::MountPoint(const QString &mountPoint)
    : m_mountPoint(mountPoint)
{
}

QString MountPoint::data() const
{
    // Done here because canonicalFilePath relies on file existence
    return normalizePath(m_mountPoint);
}

} // namespace PlasmaVault

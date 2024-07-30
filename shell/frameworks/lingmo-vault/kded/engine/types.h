/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef LINGMOVAULT_KDED_ENGINE_COMMON_TYPES_H
#define LINGMOVAULT_KDED_ENGINE_COMMON_TYPES_H

#include <QHash>
#include <QString>

#define LINGMOVAULT_CONFIG_FILE QStringLiteral("lingmovaultrc")

namespace LingmoVault
{
class Device
{
public:
    explicit Device(const QString &device = QString());
    QString data() const;

private:
    QString m_device;
};

inline uint qHash(const Device &value, uint seed = 0)
{
    return qHash(value.data(), seed);
}

inline bool operator==(const Device &left, const Device &right)
{
    return left.data() == right.data();
}

class MountPoint
{
public:
    explicit MountPoint(const QString &mountPoint = QString());
    QString data() const;

    inline bool isEmpty() const
    {
        return m_mountPoint.isEmpty();
    }

private:
    QString m_mountPoint;
};

} // namespace LingmoVault

#endif // include guard

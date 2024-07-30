/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_BATTERY_P_H
#define BLUEZQT_BATTERY_P_H

#include <QObject>

#include "battery.h"

namespace BluezQt
{
class BatteryPrivate : public QObject
{
    Q_OBJECT

public:
    explicit BatteryPrivate(const QString &path, const QVariantMap &properties);

    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    QWeakPointer<Battery> q;

    QString m_path;
    int m_percentage = 0;
};

} // namespace BluezQt

#endif // BLUEZQT_BATTERY_P_H

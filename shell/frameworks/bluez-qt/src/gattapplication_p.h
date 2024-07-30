/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTAPPLICATION_P_H
#define BLUEZQT_GATTAPPLICATION_P_H

#include "bluezqt_dbustypes.h"

#include <QDBusObjectPath>

namespace BluezQt
{
class GattApplication;

class GattApplicationPrivate
{
public:
    GattApplicationPrivate(const QString &objectPathPrefix, GattApplication *q_ptr);

    DBusManagerStruct getManagedObjects() const;

    GattApplication *q;
    QDBusObjectPath m_objectPath;
};

} // namespace BluezQt

#endif

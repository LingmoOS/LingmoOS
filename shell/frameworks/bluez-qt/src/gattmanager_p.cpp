/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattmanager_p.h"
#include "utils.h"

namespace BluezQt
{
GattManagerPrivate::GattManagerPrivate(const QString &path)
    : m_dbusInterface(Strings::orgBluez(), path, DBusConnection::orgBluez())
{
}

GattManagerPrivate::~GattManagerPrivate()
{
}

} // namespace BluezQt

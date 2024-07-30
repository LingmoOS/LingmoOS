/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "objectmanageradaptor.h"
#include "gattapplication.h"
#include "gattapplication_p.h"

namespace BluezQt
{
ObjectManagerAdaptor::ObjectManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_gattApplication(qobject_cast<GattApplication *>(parent))
{
}

DBusManagerStruct ObjectManagerAdaptor::GetManagedObjects()
{
    if (m_gattApplication) {
        return m_gattApplication->d->getManagedObjects();
    }

    return {};
}

} // namespace BluezQt

#include "moc_objectmanageradaptor.cpp"

/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBJECTMANAGERADAPTOR_H
#define BLUEZQT_OBJECTMANAGERADAPTOR_H

#include <QDBusAbstractAdaptor>

#include "bluezqt_dbustypes.h"

namespace BluezQt
{
class GattApplication;

class ObjectManagerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.ObjectManager")

public:
    explicit ObjectManagerAdaptor(QObject *parent);

public Q_SLOTS:
    DBusManagerStruct GetManagedObjects();

Q_SIGNALS:
    void InterfacesAdded(const QDBusObjectPath &object, const QVariantMapMap &interfaces);
    void InterfacesRemoved(const QDBusObjectPath &object, const QStringList &interfaces);

private:
    GattApplication *m_gattApplication = nullptr;
};

} // namespace BluezQt

#endif

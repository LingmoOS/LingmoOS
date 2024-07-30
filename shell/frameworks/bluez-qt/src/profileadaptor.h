/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_PROFILEADAPTOR_H
#define BLUEZQT_PROFILEADAPTOR_H

#include <QDBusAbstractAdaptor>

class QDBusMessage;
class QDBusObjectPath;
class QDBusUnixFileDescriptor;

namespace BluezQt
{
class Manager;
class Profile;

class ProfileAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Profile1")

public:
    explicit ProfileAdaptor(Profile *parent, Manager *manager);

public Q_SLOTS:
    void NewConnection(const QDBusObjectPath &device, const QDBusUnixFileDescriptor &fd, const QVariantMap &properties, const QDBusMessage &msg);
    void RequestDisconnection(const QDBusObjectPath &device, const QDBusMessage &msg);
    Q_NOREPLY void Release();

private:
    Profile *m_profile;
    Manager *m_manager;
};

} // namespace BluezQt

#endif // BLUEZQT_PROFILEADAPTOR_H

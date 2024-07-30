/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2018 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AVAHILISTENER_H
#define AVAHILISTENER_H

#include <QDBusMessage>
#include <QString>

namespace KDNSSD
{
// Assists with listening to Avahi for all signals and then checking if the
// a given dbus message is meant for us or not.
// Subclass and set the object path to object path you should be listening to.
// Messages may then be run through isOurMsg to determine if they target the
// object at hand.
class AvahiListener
{
public:
    explicit AvahiListener();
    virtual ~AvahiListener();

    // This method gets called a lot but doesn't do much. Suggest inlining.
    inline bool isOurMsg(const QDBusMessage &msg) const
    {
        if (m_dbusObjectPath.isEmpty() || m_dbusObjectPath != msg.path()) {
            return false;
        }
        return true;
    }

    QString m_dbusObjectPath; // public so !Private objects can access it
};

} // namespace KDNSSD

#endif // AVAHILISTENER_H

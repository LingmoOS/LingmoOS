/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef _KWALLETDBUSCONTEXT_H_
#define _KWALLETDBUSCONTEXT_H_

#include "qdbuserror.h"
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>

/* FDO_DBUS_CONTEXT macro will be replaced by KWalletDBusContextDummy during
 * preprocessing if FDO_ENABLE_DUMMY_MESSAGE_CONNECTION was defined,
 * otherwise we get QDBusContext.
 *
 * This is used for mocking QDBusContext in autotests.
 *
 * QDBusContext's connection() and message() member functions can't be called
 * without a real connection context (this cause segfault).
 * So we need to use KWalletDBusContextDummy in case some DBus-related
 * member functions may call connection()/message().
 *
 * This header defines FDO_DBUS_CONTEXT macro that should be used instead of
 * QDBusContext in all DBus-related which we want to use in autotests.
 */

#ifdef FDO_ENABLE_DUMMY_MESSAGE_CONNECTION

class KWalletDBusContextDummy : public QDBusContext
{
public:
    const QDBusMessage &message()
    {
        static auto msg = QDBusMessage::createSignal(QStringLiteral("dummy"), QStringLiteral("dummy"), QStringLiteral("dummy"));
        return msg;
    }
    QDBusConnection connection() const
    {
        return QDBusConnection::sessionBus();
    }
};

#define FDO_DBUS_CONTEXT KWalletDBusContextDummy

#else

#define FDO_DBUS_CONTEXT QDBusContext

#endif

#endif // _KWALLETDBUSCONTEXT_H_

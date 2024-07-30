/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2022 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kwalletdbuscontext.h"

const QDBusMessage &KWalletDBusContext::message() const
{
    return QDBusContext::message();
}

QDBusConnection KWalletDBusContext::connection() const
{
    return QDBusContext::connection();
}

/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "virtualkeyboard.h"

KwinVirtualKeyboardInterface::KwinVirtualKeyboardInterface()
    : OrgKdeKwinVirtualKeyboardInterface(QStringLiteral("org.kde.KWin"), QStringLiteral("/VirtualKeyboard"), QDBusConnection::sessionBus())
{
}

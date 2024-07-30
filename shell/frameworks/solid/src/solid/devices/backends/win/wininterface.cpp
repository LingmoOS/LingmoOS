/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wininterface.h"

using namespace Solid::Backends::Win;

WinInterface::WinInterface(WinDevice *device)
    : QObject(device)
    , m_device(device)
{
}

WinInterface::~WinInterface()
{
}

#include "moc_wininterface.cpp"

/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2009 Maciej Mrozowski <reavertm@poczta.fm>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

// Test the case where Xdefs.h is first
#include <X11/Xdefs.h>

static Bool foo()
{
    return 1; // Xdefs doesn't define True!
}

#include <X11/Xlib.h>
#include <fixx11h.h>

int main(int, char **)
{
    Bool b = foo();
    return b;
}

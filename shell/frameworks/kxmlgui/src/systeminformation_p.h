/* This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef SYSTEMINFORMATION_P_H
#define SYSTEMINFORMATION_P_H

#include <QString>

namespace SystemInformation
{
QString userName();
}

#if !defined(Q_OS_WIN)
#include <pwd.h>
#include <sys/utsname.h>
#include <unistd.h>
inline QString SystemInformation::userName()
{
    struct passwd *p = getpwuid(getuid());
    return QString::fromLatin1(p->pw_name);
}

#else
#include <QOperatingSystemVersion>
#include <qt_windows.h>
#define SECURITY_WIN32
#include <security.h>
//#include <secext.h> // GetUserNameEx

inline QString SystemInformation::userName()
{
    WCHAR nameBuffer[256];
    DWORD bufsize = 256;
    if (!GetUserNameExW(NameDisplay, nameBuffer, &bufsize)) {
        return QStringLiteral("Unknown User"); // should never happen (translate?)
    }
    return QString::fromWCharArray(nameBuffer);
}

#endif

#endif // SYSTEMINFORMATION_P_H

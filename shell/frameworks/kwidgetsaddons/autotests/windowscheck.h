/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2024 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

inline bool isWindowsCI()
{
#ifdef Q_OS_WIN
    return qEnvironmentVariable("CI") == QLatin1String("true");
#else
    return false;
#endif
}

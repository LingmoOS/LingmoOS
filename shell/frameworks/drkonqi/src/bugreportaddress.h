/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BUGREPORTADDRESS_H
#define BUGREPORTADDRESS_H

#include <QString>

#include "drkonqi_globals.h"

class BugReportAddress : public QString
{
public:
    inline BugReportAddress()
        : QString()
    {
    }
    inline BugReportAddress(const QString &address)
        : QString(address == QLatin1String("submit@bugs.kde.org") ? KDE_BUGZILLA_URL : address)
    {
    }

    inline bool isKdeBugzilla() const
    {
        return *this == KDE_BUGZILLA_URL;
    }

    inline bool isEmail() const
    {
        return contains(QLatin1Char('@'));
    }
};

#endif

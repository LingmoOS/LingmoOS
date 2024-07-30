/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DRKONQI_GLOBALS_H
#define DRKONQI_GLOBALS_H

#include <KGuiItem>
#include <QIcon>

#include "drkonqi.h"

/** This class provides a custom constructor to fill the "toolTip"
 * and "whatsThis" texts of KGuiItem with the same text.
 */
class KGuiItem2 : public KGuiItem
{
public:
    inline KGuiItem2(const QString &text, const QIcon &icon, const QString &toolTip)
        : KGuiItem(text, icon, toolTip, toolTip)
    {
    }
};

namespace DrStandardGuiItem
{
KGuiItem2 appRestart();
}; // namespace DrStandardGuiItem

/* Urls are defined globally here, so that they can change easily */
#define KDE_BUGZILLA_URL DrKonqi::kdeBugzillaURL()
#define KDE_BUGZILLA_CREATE_ACCOUNT_URL KDE_BUGZILLA_URL + QStringLiteral("createaccount.cgi")
#define KDE_BUGZILLA_SHORT_URL "bugs.kde.org"
#define TECHBASE_HOWTO_DOC "https://community.kde.org/Guidelines_and_HOWTOs/Debugging/How_to_create_useful_crash_reports#Preparing_your_KDE_packages"

#endif

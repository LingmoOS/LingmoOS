// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

pragma Singleton
import QtQuick 2.6

import org.kde.drkonqi 1.0

QtObject {
    readonly property string bugzillaUrl: DrKonqi.kdeBugzillaURL()
    readonly property string bugzillaCreateAccountUrl: bugzillaUrl + "createaccount.cgi"
    readonly property string ownBugzillaUrl: bugzillaUrl + "enter_bug.cgi?product=drkonqi&format=guided"
    readonly property string bugzillaShortUrl: DrKonqi.kdeBugzillaDomain()
    readonly property string techbaseHowtoDoc: "https://community.kde.org/Guidelines_and_HOWTOs/Debugging/How_to_create_useful_crash_reports#Preparing_your_KDE_packages"
    readonly property string aboutBugReportingUrl: "https://community.kde.org/Get_Involved/Issue_Reporting"

    function bugUrl(bugNumber) {
        return bugzillaUrl + "show_bug.cgi?id=" + bugNumber
    }
}

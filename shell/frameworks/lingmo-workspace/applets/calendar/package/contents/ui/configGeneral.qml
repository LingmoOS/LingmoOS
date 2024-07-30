/*
    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QtControls
import org.kde.lingmoui 2.5 as LingmoUI
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_showWeekNumbers: showWeekNumbers.checked
    property string cfg_compactDisplay

    LingmoUI.FormLayout {
        QtControls.CheckBox {
            id: showWeekNumbers

            LingmoUI.FormData.label: i18n("Calendar version:")

            text: i18n("Show week numbers")
        }

        Item {
            LingmoUI.FormData.isSection: true
        }


        QtControls.RadioButton {
            LingmoUI.FormData.label: i18nc("What information is shown in the calendar icon", "Compact version:")

            text: i18nc("Show the number of the day (eg. 31) in the icon", "Show day of the month")

            checked: cfg_compactDisplay == "d"
            onCheckedChanged: if (checked) cfg_compactDisplay = "d"
        }
        QtControls.RadioButton {
            text: i18nc("Show the week number (eg. 50) in the icon", "Show week number")

            checked: cfg_compactDisplay == "w"
            onCheckedChanged: if (checked) cfg_compactDisplay = "w"
        }
    }
}

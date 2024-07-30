/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>
 *   SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 *   SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import org.kde.lingmo.quicksetting.nightcolor 1.0 as NightColor
import org.kde.lingmo.private.mobileshell.quicksettingsplugin as QS

QS.QuickSetting {
    text: i18n("Night Color")
    icon: "redshift-status-on"
    enabled: NightColor.NightColorUtil.enabled
    status: ""
    settingsCommand: "lingmo-open-settings kcm_nightcolor"

    function toggle() {
        NightColor.NightColorUtil.enabled = !enabled;
    }
}

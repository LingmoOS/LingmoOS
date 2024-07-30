/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.kirigami as Kirigami


Kirigami.FormLayout {
    id: root

    property alias cfg_showDetails: showDetailsCheckbox.checked
    property alias cfg_askWhenKilling: confirmCheckbox.checked

    CheckBox {
        id: showDetailsCheckbox
        text: i18nc("@option:check", "Show details panel")
    }

    CheckBox {
        id: confirmCheckbox
        text: i18nc("@option:check", "Confirm when quitting applications")
    }
}

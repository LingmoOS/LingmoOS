/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Kirigami.FormLayout {
    id: root

    property alias cfg_groupByTotal: groupCheckbox.checked

    Controls.CheckBox {
        id: groupCheckbox
        Layout.fillWidth: true
        text: i18n("Group sensors based on the value of the total sensors.")
    }
}


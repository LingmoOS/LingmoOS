/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

QQC2.Pane {
    id: root

    property alias cfg_BoolTest: testBoolConfigField.checked
    property alias cfg_Test: testConfigField.text
    property alias cfg_IntTest: intTestConfigField.value

    LingmoUI.FormLayout {
        anchors.fill: parent

        QQC2.CheckBox {
            id: testBoolConfigField
            text: i18nc("@label example config", "Bool from config")
            LingmoUI.FormData.label: i18nc("@label example config", "Bool Config value:")
        }
        QQC2.TextField {
            id: testConfigField
            placeholderText: i18nc("@label example config", "String test")
            LingmoUI.FormData.label: i18nc("@label example config", "Text Config value:")
        }
        QQC2.SpinBox {
            id: intTestConfigField
            LingmoUI.FormData.label: i18nc("@label example config", "Integer:")
            from: -1
            to: 100
        }
    }
}

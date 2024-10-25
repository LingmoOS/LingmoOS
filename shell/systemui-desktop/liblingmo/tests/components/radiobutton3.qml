/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 George Vogiatzis <Gvgeo@protonmail.com>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 RadioButton"
    contentItem: GridLayout {
        columnSpacing: LingmoUI.Units.gridUnit
        rowSpacing: LingmoUI.Units.smallSpacing
        columns: 2

        LingmoComponents.Label {
            text: "text"
        }
        LingmoComponents.RadioButton {
            text: "Some awesome radiobutton"
        }

        LingmoComponents.Label {
            text: "focus"
        }
        LingmoComponents.RadioButton {
            text: "Some awesome radiobutton"
            focus: true
        }

        LingmoComponents.Label {
            text: "checked"
        }
        LingmoComponents.RadioButton {
            text: "Some awesome radiobutton"
            checked: true
        }

        LingmoComponents.Label {
            text: "disabled"
        }
        LingmoComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
        }

        LingmoComponents.Label {
            text: "disabled and checked"
        }
        LingmoComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
            checked: true
        }
    }
}


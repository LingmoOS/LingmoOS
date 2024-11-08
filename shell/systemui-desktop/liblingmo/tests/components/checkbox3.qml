/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 CheckBox"
    contentItem: GridLayout {
        columnSpacing: LingmoUI.Units.gridUnit
        rowSpacing: LingmoUI.Units.smallSpacing
        columns: 2

        LingmoComponents.Label {
            text: "text"
        }
        LingmoComponents.CheckBox {
            text: "Some awesome checkbox"
        }

        LingmoComponents.Label {
            text: "icon"
        }
        LingmoComponents.CheckBox {
            icon.name: "start-here-kde-lingmo"
        }

        LingmoComponents.Label {
            text: "text plus icon"
        }
        LingmoComponents.CheckBox {
            text: "text"
            icon.name: "start-here-kde-lingmo"
        }

        LingmoComponents.Label {
            text: "focus"
        }
        LingmoComponents.CheckBox {
            text: "Some awesome checkbox"
            focus: true
        }

        LingmoComponents.Label {
            text: "checked"
        }
        LingmoComponents.CheckBox {
            text: "Some awesome checkbox"
            checkState: Qt.Checked
        }

        LingmoComponents.Label {
            text: "tri-state"
        }
        LingmoComponents.CheckBox {
            text: "Some awesome checkbox"
            checkState: Qt.PartiallyChecked
        }

        LingmoComponents.Label {
            text: "disabled"
        }
        LingmoComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
        }

        LingmoComponents.Label {
            text: "disabled and checked"
        }
        LingmoComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
            checkState: Qt.Checked
        }
    }
}


/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

ColumnLayout {
    id: root

    // Screen layouts model.
    //
    // type: [{
    //  iconName: string,
    //  label: string,
    //  action: enum<OsdAction::Action>,
    // }]
    property var screenLayouts

    spacing: Kirigami.Units.smallSpacing * 2

    states: [
        State {
            // only makes sense to offer screen layout setup if there's more than one screen connected
            when: Plasmoid.connectedOutputCount < 2

            PropertyChanges {
                target: screenLayoutRow
                enabled: false
            }
            PropertyChanges {
                target: noScreenLabel
                visible: true
            }
        }
    ]

    Kirigami.Heading {
        Layout.fillWidth: true
        level: 3
        text: i18n("Screen Layout")
    }

    // Screen layout selector section
    Row {
        id: screenLayoutRow
        readonly property int buttonSize: Math.floor((width - spacing * (screenLayoutRepeater.count - 1)) / screenLayoutRepeater.count)
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            id: screenLayoutRepeater
            model: root.screenLayouts

            PlasmaComponents3.Button {
                id: screenLayoutDelegate

                required property /*KScreen.OsdAction*/var modelData

                width: screenLayoutRow.buttonSize
                height: screenLayoutRow.buttonSize

                display: T.Button.IconOnly
                icon.name: modelData.iconName
                icon.width: availableWidth
                icon.height: availableHeight
                text: modelData.label

                onClicked: Plasmoid.applyLayoutPreset(modelData.action)

                Accessible.name: text
                PlasmaComponents3.ToolTip { text: screenLayoutDelegate.text }
            }
        }
    }

    PlasmaExtras.DescriptiveLabel {
        id: noScreenLabel
        Layout.fillWidth: true
        Layout.maximumWidth: Math.min(Kirigami.Units.gridUnit * 20, implicitWidth)
        wrapMode: Text.Wrap
        text: i18n("You can only apply a different screen layout when there is more than one display device plugged in.")
        font: Kirigami.Theme.smallFont
        visible: false
    }
}

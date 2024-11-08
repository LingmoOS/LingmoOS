/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.extras as LingmoExtras
import org.kde.lingmo.components as PC3
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root

    title: "Lingmo Extras Menu Placement"

    LingmoExtras.Menu {
        id: menu

        visualParent: centralButton

        placement: LingmoExtras.Menu.BottomPosedRightAlignedPopup
        preferSeamlessEdges: false

        LingmoExtras.MenuItem { text: "Hello"; section: true }
        LingmoExtras.MenuItem { text: "This is just a simple" }
        LingmoExtras.MenuItem { text: "Menu" }
        LingmoExtras.MenuItem { separator: true }
        LingmoExtras.MenuItem { text: "With separators" }
        LingmoExtras.MenuItem { text: "And other stuff" }
    }

    component PlacementRadioButton : PC3.RadioButton {
        required property string placementName

        anchors.margins: LingmoUI.Units.gridUnit
        text: placementName.replace("Popup", "").replace(/([a-z])([A-Z])/g, "$1 $2")
        checked: menu.placement === LingmoExtras.Menu[placementName]
        onToggled: menu.placement = LingmoExtras.Menu[placementName]
    }

    contentItem: Item {
        implicitWidth: 600
        implicitHeight: 500

        PC3.Button {
            id: centralButton
            anchors.centerIn: parent
            width: 200
            height: 200
            text: "Open Relative"
            onClicked: {
                menu.openRelative()
            }
        }

        Repeater {
            model: [
                { placementName: "TopPosedLeftAlignedPopup", bottom: "top", right: "horizontalCenter" },
                { placementName: "TopPosedRightAlignedPopup", bottom: "top", left: "horizontalCenter" },
                { placementName: "LeftPosedTopAlignedPopup", right: "left", bottom: "verticalCenter" },
                { placementName: "LeftPosedBottomAlignedPopup", right: "left", top: "verticalCenter" },
                { placementName: "BottomPosedLeftAlignedPopup", top: "bottom", right: "horizontalCenter" },
                { placementName: "BottomPosedRightAlignedPopup", top: "bottom", left: "horizontalCenter"  },
                { placementName: "RightPosedTopAlignedPopup", left: "right", bottom: "verticalCenter" },
                { placementName: "RightPosedBottomAlignedPopup", left: "right", top: "verticalCenter" },
            ]
            PlacementRadioButton {
                required property var modelData
                placementName: modelData.placementName
                anchors.top: centralButton[modelData.top]
                anchors.left: centralButton[modelData.left]
                anchors.right: centralButton[modelData.right]
                anchors.bottom: centralButton[modelData.bottom]
            }
        }
    }

    footer: PC3.ToolBar {
        contentItem: RowLayout {
            spacing: LingmoUI.Units.smallSpacing

            PC3.CheckBox {
                Layout.alignment: Qt.AlignHCenter
                text: "Prefer Seamless Edges"
                checked: menu.preferSeamlessEdges
                onToggled: menu.preferSeamlessEdges = checked;
            }
        }
    }
}

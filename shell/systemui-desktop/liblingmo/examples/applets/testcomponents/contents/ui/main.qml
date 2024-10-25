/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.plasmoid

PlasmoidItem {
    id: root

    property int currentIndex: 0
    onCurrentIndexChanged: print("AAA", currentIndex)

    fullRepresentation: ColumnLayout {
        spacing: 0

        Layout.minimumWidth: LingmoUI.Units.gridUnit * 15
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 15

        Layout.maximumWidth: LingmoUI.Units.gridUnit * 60
        Layout.maximumHeight: LingmoUI.Units.gridUnit * 60

        LingmoComponents.TabBar {
            id: tabBar

            currentIndex: root.currentIndex

            onCurrentIndexChanged: {
                root.currentIndex = currentIndex;
            }

            Layout.fillWidth: true
            // Layout.preferredHeight:
            // anchors {
            //     left: parent.left
            //     right: parent.right
            //     top: parent.top
            // }

            // height: LingmoUI.Units.iconSizes.desktop

            LingmoComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Theme Page"
                icon.name: "preferences-desktop-appearance"
            }
            LingmoComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Drag Page"
                icon.name: "preferences-desktop-mouse"
            }
            LingmoComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Icons Page"
                icon.name: "preferences-desktop-icons"
            }
            LingmoComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Dialogs Page"
                icon.name: "preferences-system-windows"
            }
            LingmoComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Buttons Page"
                icon.name: "preferences-desktop-theme"
            }
            LingmoComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Plasmoid Page"
                icon.name: "lingmo"
            }
            LingmoComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Mouse Page"
                icon.name: "preferences-desktop-mouse"
            }
        }

        LingmoComponents.SwipeView {
            id: contentViewContainer

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true

            currentIndex: root.currentIndex

            onCurrentIndexChanged: {
                root.currentIndex = currentIndex;
            }

            ThemePage {}
            DragPage {}
            // IconsPage {
            //     id: iconsPage
            // }
            // DialogsPage {
            //     id: dialogsPage
            // }
            // ButtonsPage {
            //     id: buttonsPage
            // }
            // PlasmoidPage {
            //     id: plasmoidPage
            // }

            // MousePage {
            //     id: mousePage
            // }

        }
    }
}

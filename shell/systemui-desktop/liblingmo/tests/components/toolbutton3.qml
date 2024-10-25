/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Marco Martin <notmart@gmail.com>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 ToolButton"

    contentItem: ColumnLayout {
        Flow {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: LingmoUI.Units.gridUnit

            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
                text: "test"
                flat: true
            }
            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
                flat: true
            }
            LingmoComponents.ToolButton {
                text: "test"
                flat: true
            }
            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
                text: "test"
                flat: false
            }
            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
                flat: false
            }
            LingmoComponents.ToolButton {
                text: "test"
                flat: false
            }
            LingmoComponents.ToolButton {
                icon.name: "application-menu"
                text: "Icon Only"
                display: LingmoComponents.ToolButton.IconOnly
            }
            LingmoComponents.ToolButton {
                icon.name: "application-menu"
                text: "Text Beside Icon"
                display: LingmoComponents.ToolButton.TextBesideIcon
            }
            LingmoComponents.ToolButton {
                icon.name: "application-menu"
                text: "Text Under Icon"
                display: LingmoComponents.ToolButton.TextUnderIcon
            }
            LingmoComponents.ToolButton {
                icon.name: "application-menu"
                text: "Text Only"
                display: LingmoComponents.ToolButton.TextOnly
            }
        }
        RowLayout {
            Layout.fillWidth: true
            LingmoComponents.Label {
                Layout.fillWidth: true
                text: "They should always be square:"
            }
            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
            }
            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
            }
            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
            }
            LingmoComponents.ToolButton {
                icon.name: "start-here-kde-lingmo"
            }
        }
        LingmoComponents.Label {
            text: "Fixed size and stretching size buttons"
        }
        GridLayout {
            id: layout
            rows: 2
            columns:2
            Layout.fillWidth: true
            Layout.fillHeight: true
            LingmoComponents.ToolButton {
                id: closeButton
                icon.name: "window-close"
                text: "Text"
            }
            LingmoComponents.ToolButton {
                id: closeButton2
                icon.name: "window-close"
                Layout.fillWidth: true
                Layout.fillHeight: true
                icon.width: LingmoUI.Units.iconSizes.small
                icon.height: LingmoUI.Units.iconSizes.small
                text: "Text"
            }
            LingmoComponents.ToolButton {
                id: closeButton3
                icon.name: "window-close"
                Layout.fillHeight: true
                text: "Text"
            }
            LingmoComponents.ToolButton {
                id: closeButton4
                icon.name: "window-close"
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: "Text"
            }
        }
    }
}


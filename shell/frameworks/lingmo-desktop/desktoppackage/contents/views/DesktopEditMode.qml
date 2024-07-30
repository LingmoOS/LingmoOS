/*
    SPDX-FileCopyrightText: 2024 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Effects
import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as PC
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.kcmutils as KCM

Item {
    property real centerX: Math.round(editModeUi.x + editModeUi.width/2)
    property real centerY: Math.round(editModeUi.y + editModeUi.height/2)
    property real roundedRootWidth: Math.round(root.width)
    property real roundedRootHeight: Math.round(root.height)

    property bool open: false
    Component.onCompleted: {
        open = Qt.binding(() => {return containment.plasmoid.corona.editMode})
    }

    // Those 2 elements have the same parameters as the overview effect
    MultiEffect {
        source: containment
        anchors.fill: parent
        blurEnabled: true
        blurMax: 64
        blur: 1.0
    }
    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.backgroundColor
        opacity: 0.7
    }

    Item {
        id: editModeUi
        visible: open || xAnim.running
        x: Math.round(open ? editModeRect.x + editModeRect.width/2 - zoomedWidth/2 : 0)
        y: Math.round(open ? editModeRect.y + editModeRect.height/2 - zoomedHeight/2 + toolBar.height/2 : 0)
        width: open ? zoomedWidth : roundedRootWidth
        height: open ? zoomedHeight : roundedRootHeight
        property real zoomedWidth: Math.round(root.width * containmentParent.scaleFactor)
        property real zoomedHeight: Math.round(root.height * containmentParent.scaleFactor)

        LingmoUI.ShadowedRectangle {
            color: LingmoUI.Theme.backgroundColor
            width: Math.round(parent.width)
            height: Math.round(parent.height + toolBar.height + LingmoUI.Units.largeSpacing)
            y: - toolBar.height - LingmoUI.Units.largeSpacing

            radius: open ? LingmoUI.Units.cornerRadius : 0
            Behavior on radius {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            shadow {
                size: LingmoUI.Units.gridUnit * 2
                color: Qt.rgba(0, 0, 0, 0.3)
                yOffset: 3
            }
            RowLayout {
                id: toolBar
                LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
                LayoutMirroring.childrenInherit: true
                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right
                    margins: LingmoUI.Units.smallSpacing
                }
                Flow {
                    Layout.fillWidth: true
                    Layout.minimumHeight: implicitHeight
                    PC.ToolButton {
                        id: addWidgetButton
                        property QtObject qAction: containment?.plasmoid.internalAction("add widgets") || null
                        text: qAction?.text
                        icon.name: "list-add"
                        onClicked: qAction.trigger()
                    }

                    PC.ToolButton {
                        id: addPanelButton
                        height: addWidgetButton.height
                        property QtObject qAction: containment?.plasmoid.corona.action("add panel") || null
                        text: qAction?.text
                        icon.name: "list-add"
                        Accessible.role: Accessible.ButtonMenu
                        onClicked: containment.plasmoid.corona.showAddPanelContextMenu(mapToGlobal(0, height))
                    }

                    PC.ToolButton {
                        id: configureButton
                        property QtObject qAction: containment?.plasmoid.internalAction("configure") || null
                        text: qAction?.text
                        icon.name: "preferences-desktop-wallpaper"
                        onClicked: qAction.trigger()
                    }

                    PC.ToolButton {
                        id: themeButton
                        text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Global Themes")
                        icon.name: "preferences-desktop-theme-global"
                        onClicked: KCM.KCMLauncher.openSystemSettings("kcm_lookandfeel")
                    }

                    PC.ToolButton {
                        id: displaySettingsButton
                        text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Display Configuration")
                        icon.name: "preferences-desktop-display"
                        onClicked: KCM.KCMLauncher.openSystemSettings("kcm_kscreen")
                    }

                    PC.ToolButton {
                        id: manageContainmentsButton
                        property QtObject qAction: containment?.plasmoid.corona.action("manage-containments") || null
                        text: qAction?.text
                        visible: qAction?.visible || false
                        icon.name: "preferences-system-windows-effect-fadedesktop"
                        onClicked: qAction.trigger()
                    }
                }

                PC.ToolButton {
                    Layout.alignment: Qt.AlignTop

                    visible: LingmoUI.Settings.hasTransientTouchInput || LingmoUI.Settings.tabletMode

                    icon.name: "overflow-menu"
                    text: i18ndc("lingmo_shell_org.kde.lingmo.desktop", "@action:button", "More")

                    onClicked: {
                        containment.openContextMenu(mapToGlobal(0, height));
                    }
                }
                PC.ToolButton {
                    Layout.alignment: Qt.AlignTop
                    icon.name: "window-close"
                    text: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Exit Edit Mode")
                    onClicked: containment.plasmoid.corona.editMode = false
                }
            }
        }

        Behavior on x {
            NumberAnimation {
                id: xAnim
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            NumberAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            NumberAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        MultiEffect {
            anchors.fill: parent
            source: containment
            layer.enabled: true
            layer.smooth: true
            layer.effect: LingmoUI.ShadowedTexture {
                width: roundedRootWidth
                height: roundedRootHeight
                color: "transparent"

                radius: open ? LingmoUI.Units.cornerRadius : 0
                Behavior on radius {
                    NumberAnimation {
                        duration: LingmoUI.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }
}

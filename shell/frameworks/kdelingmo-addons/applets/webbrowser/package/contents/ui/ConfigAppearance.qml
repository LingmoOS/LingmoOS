/*
 *  SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.iconthemes as KIconThemes
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.ksvg 1.0 as KSvg
import org.kde.kcmutils as KCM

import org.kde.lingmo.core as LingmoCore

KCM.SimpleKCM {
    property string cfg_icon: plasmoid.configuration.icon
    property alias cfg_useFavIcon: useFavIcon.checked
    property alias cfg_enableNavigationBar: enableNavigationBar.checked


    LingmoUI.FormLayout {

        QQC2.ButtonGroup {
            id: iconGroup
        }

        QQC2.RadioButton {
            id: useFavIcon

            LingmoUI.FormData.label: i18nc("@title:group", "Icon:")
            text: i18nc("@option:radio", "Website's favicon")

            QQC2.ButtonGroup.group: iconGroup
        }

        RowLayout {
            spacing: LingmoUI.Units.smallSpacing

            QQC2.RadioButton {
                id: useSystemIcon
                checked: !cfg_useFavIcon

                QQC2.ButtonGroup.group: iconGroup
            }

            QQC2.Button {
                id: iconButton

                implicitWidth: previewFrame.width + LingmoUI.Units.smallSpacing * 2
                implicitHeight: previewFrame.height + LingmoUI.Units.smallSpacing * 2
                enabled: useSystemIcon.checked
                hoverEnabled: true

                Accessible.name: i18nc("@action:button", "Change Web Browser's icon")
                Accessible.description: i18nc("@info:whatsthis", "Current icon is %1. Click to open menu to change the current icon or reset to the default icon.", cfg_icon)
                Accessible.role: Accessible.ButtonMenu

                QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay
                QQC2.ToolTip.text: i18nc("@info:tooltip", "Icon name is \"%1\"", cfg_icon)
                QQC2.ToolTip.visible: iconButton.hovered && cfg_icon.length > 0

                onPressed: iconMenu.opened ? iconMenu.close() : iconMenu.open()

                KIconThemes.IconDialog {
                    id: iconDialog
                    onIconNameChanged: {
                        cfg_icon = iconName || "";
                    }
                }

                KSvg.FrameSvgItem {
                    id: previewFrame
                    anchors.centerIn: parent
                    imagePath: plasmoid.formFactor === LingmoCore.Types.Vertical || plasmoid.formFactor === LingmoCore.Types.Horizontal
                            ? "widgets/panel-background" : "widgets/background"
                    width: LingmoUI.Units.iconSizes.large + fixedMargins.left + fixedMargins.right
                    height: LingmoUI.Units.iconSizes.large + fixedMargins.top + fixedMargins.bottom

                    LingmoUI.Icon {
                        anchors.centerIn: parent
                        width: LingmoUI.Units.iconSizes.large
                        height: width
                        source: cfg_icon || "applications-internet"
                    }
                }

                QQC2.Menu {
                    id: iconMenu

                    // Appear below the button
                    y: +parent.height

                    QQC2.MenuItem {
                        text: i18nc("@item:inmenu Open icon chooser dialog", "Choose…")
                        icon.name: "document-open-folder"
                        Accessible.description: i18nc("@info:whatsthis", "Choose an icon for Web Browser")
                        onClicked: iconDialog.open()
                    }
                    QQC2.MenuItem {
                        text: i18nc("@item:inmenu Reset icon to default", "Reset to default icon")
                        icon.name: "edit-clear"
                        enabled: cfg_icon !== ""
                        onClicked: cfg_icon = ""
                    }
                }
            }
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: enableNavigationBar
            text: i18nc("@option:check", "Display Navigation Bar")
            checked: enableNavigationBar.checked

            onCheckedChanged: {
                cfg_enableNavigationBar = enableNavigationBar.checked
            }
        }
    }
}

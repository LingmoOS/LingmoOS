/*
    SPDX-FileCopyrightText: 2020 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCM

QQC2.ItemDelegate {
    id: ruleDelegate

    width: ListView.view.width
    highlighted: false
    hoverEnabled: false
    down: false

    contentItem: RowLayout {

        LingmoUI.Icon {
            id: itemIcon
            source: model.icon
            Layout.preferredHeight: LingmoUI.Units.iconSizes.smallMedium
            Layout.preferredWidth: LingmoUI.Units.iconSizes.smallMedium
            Layout.rightMargin: LingmoUI.Units.smallSpacing
            Layout.alignment: Qt.AlignVCenter
        }

        RowLayout {
            Layout.preferredWidth: 10 * LingmoUI.Units.gridUnit
            Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
            spacing: LingmoUI.Units.smallSpacing

            QQC2.Label {
                id: label
                text: model.name
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                HoverHandler {
                    id: labelHover
                    enabled: label.truncated
                }

                QQC2.ToolTip.text: model.name
                QQC2.ToolTip.visible: labelHover.hovered
                QQC2.ToolTip.delay: LingmoUI.Units.toolTipDelay
            }

            LingmoUI.ContextualHelpButton {
                Layout.alignment: Qt.AlignVCenter
                visible: model.description.length > 0
                toolTipText: model.description
            }
        }

        RowLayout {
            // This layout keeps the width constant between delegates, independent of items visibility
            Layout.fillWidth: true
            Layout.preferredWidth: 20 * LingmoUI.Units.gridUnit
            Layout.minimumWidth: 13 * LingmoUI.Units.gridUnit

            OptionsComboBox {
                id: policyCombo
                Layout.preferredWidth: 50  // 50%
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                visible: count > 0

                model: policyModel
                onActivated: {
                    policy = currentValue;
                }
            }

            ValueEditor {
                id: valueEditor
                Layout.preferredWidth: 50  // 50%
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                ruleValue: model.value
                ruleOptions: model.options
                controlType: model.type

                onValueEdited: (value) => {
                    model.value = value;
                }
            }

            QQC2.ToolButton {
                id: itemEnabled
                icon.name: "edit-delete"
                visible: model.selectable
                Layout.alignment: Qt.AlignVCenter
                onClicked: {
                    model.enabled = false;
                }
            }
        }

        QQC2.ToolTip {
            text: model.description
            visible: hovered && (text.length > 0)
        }
    }
}

/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCMUtils
import org.kde.lingmo.touchscreen.kcm as TouchScreenKCM

KCMUtils.SimpleKCM {
    id: root

    property TouchScreenKCM.InputDevice device

    KCMUtils.ConfigModule.buttons: KCMUtils.ConfigModule.Default | KCMUtils.ConfigModule.Apply

    implicitWidth: LingmoUI.Units.gridUnit * 38
    implicitHeight: LingmoUI.Units.gridUnit * 35

    LingmoUI.PlaceholderMessage {
        icon.name: "input-touchscreen"
        text: i18n("No touchscreens found")
        explanation: i18n("Connect an external touchscreen")
        anchors.centerIn: parent
        visible: combo.count === 0
        width: parent.width - (LingmoUI.Units.largeSpacing * 4)
    }

    LingmoUI.FormLayout {

        visible: combo.count > 0

        QQC2.ComboBox {
            id: combo
            model: kcm.touchscreensModel

            enabled: count > 1 //It's only interesting when there's more than 1 touchscreen

            LingmoUI.FormData.label: i18nc("@label:listbox The device we are configuring", "Device:")

            onCountChanged: if (count > 0 && currentIndex < 0) {
                currentIndex = 0;
            }

            onCurrentIndexChanged: {
                root.device = kcm.touchscreensModel.deviceAt(currentIndex)
            }
        }


        QQC2.CheckBox {
            LingmoUI.FormData.label: i18n("Enabled:")

            checked: root.device?.enabled ?? false
            onToggled: root.device.enabled = checked
        }

        QQC2.ComboBox {
            id: outputsCombo
            LingmoUI.FormData.label: i18n("Target display:")
            model: TouchScreenKCM.OutputsModel {
                id: outputsModel
            }
            enabled: count > 2 //It's only interesting when there's more than 1 screen
            currentIndex: {
                if (!root.device || count === 0) {
                    return -1;
                }

                return outputsModel.rowForOutputName(root.device.outputName);
            }
            textRole: "display"
            onActivated: {
                if (root.device) {
                    root.device.outputName = outputsModel.outputNameAt(currentIndex);
                }
            }
        }
    }
}

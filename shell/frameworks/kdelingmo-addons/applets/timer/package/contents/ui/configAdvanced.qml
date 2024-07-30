/*
 *  SPDX-FileCopyrightText: 2015 Bernhard Friedrich <friesoft@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0

import org.kde.lingmoui 2.5 as LingmoUI
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_runCommand: runCommand.checked
    property alias cfg_command: command.text

    LingmoUI.FormLayout {
        RowLayout {
            Layout.fillWidth: true

            LingmoUI.FormData.label: i18nc("@title:label", "After timer completes:")

            QQC2.CheckBox {
                id: runCommand
                text: i18nc("@option:check", "Execute command:")
                onClicked: {
                    if (checked) {
                        command.forceActiveFocus();
                    }
                }
            }

            QQC2.TextField {
                id: command
                Layout.fillWidth: true
                enabled: runCommand.checked
            }
        }
    }
}

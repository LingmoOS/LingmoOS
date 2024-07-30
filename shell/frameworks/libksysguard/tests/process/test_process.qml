/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.ksysguard.process as Process

Pane {
    width: 400
    height: 400

    ColumnLayout {
        anchors.fill: parent

        TextField {
            id: input
            Layout.fillWidth: true
            placeholderText: "PID"
        }

        ComboBox {
            id: signalCombo

            Layout.fillWidth: true

            textRole: "key"

            model: [
                { key: "Stop", value: Process.ProcessController.StopSignal },
                { key: "Continue", value: Process.ProcessController.ContinueSignal },
                { key: "Hangup", value: Process.ProcessController.HangupSignal },
                { key: "Interrupt", value: Process.ProcessController.InterruptSignal },
                { key: "Terminate", value: Process.ProcessController.TerminateSignal },
                { key: "Kill", value: Process.ProcessController.KillSignal },
                { key: "User 1", value: Process.ProcessController.User1Signal },
                { key: "User 2", value: Process.ProcessController.User2Signal }
            ]
        }

        Button {
            Layout.fillWidth: true
            text: "Send Signal"
            onClicked: {
                var signalToSend = signalCombo.model[signalCombo.currentIndex]
                print("Sending", signalToSend.key, "(%1)".arg(signalToSend.value), "to PID", parseInt(input.text))
                var result = controller.sendSignal([parseInt(input.text)], signalToSend.value);
                print("Result:", result)
                resultLabel.text = controller.resultToString(result)
            }
        }

        Label {
            id: resultLabel
            Layout.fillWidth: true
        }
    }

    Process.ProcessController {
        id: controller
    }
}

// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0


import org.kde.kirigami 2.7 as Kirigami

import PicoWizard 1.0


Item {
    property bool runningFinishHook: false

    FinishModule {
        id: finishModule

        Component.onCompleted: {
            finishModule.runScripts()
        }
    }

    ColumnLayout {
        visible: !finishModule.isComplete
        anchors.centerIn: parent

        Label {
            font.weight: Font.Light
            font.pointSize: 32
            text: qsTr("Finalizing")
            color: Kirigami.Theme.textColor
        }

        Label {
            visible: finishModule.totalScriptsCount > 0
            Layout.topMargin: 48
            Layout.alignment: Qt.AlignHCenter
            font.weight: Font.Light
            font.pointSize: 10
            text: qsTr("Running Script [%1/%2]").arg(finishModule.runningScriptIndex + 1).arg(finishModule.totalScriptsCount)
            color: Kirigami.Theme.textColor
            opacity: 0.5
        }
    }

    Label {
        visible: finishModule.isComplete
        font.weight: Font.Light
        font.pointSize: 32
        text: qsTr("Setup complete")
        anchors.centerIn: parent
        color: Kirigami.Theme.textColor
    }

    RoundButton {
        width: Kirigami.Units.iconSizes.huge
        height: Kirigami.Units.iconSizes.huge

        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 12
        }
        flat: true
        onClicked: {
            if (finishModule.isComplete) {
                runningFinishHook = true
                finishModule.runCompleteHook()
            }
        }

        background: Rectangle {
            color: finishModule.isComplete ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.highlightColor
            radius: parent.width
        }

        Kirigami.Icon {
            visible: finishModule.isComplete && !runningFinishHook
            width: Kirigami.Units.iconSizes.smallMedium
            height: Kirigami.Units.iconSizes.smallMedium
            color: Kirigami.Theme.highlightedTextColor
            isMask: true

            anchors.centerIn: parent
            source: "answer-correct"
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: !finishModule.isComplete || runningFinishHook
            width: Kirigami.Units.iconSizes.smallMedium
            height: Kirigami.Units.iconSizes.smallMedium
            Layout.alignment: Qt.AlignHCenter
        }
    }
}

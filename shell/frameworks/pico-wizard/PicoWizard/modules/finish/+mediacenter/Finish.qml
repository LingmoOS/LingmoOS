// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0


import org.kde.kirigami 2.7 as Kirigami

import PicoWizard 1.0


FocusScope {
    property bool runningFinishHook: false

    Component.onCompleted: {
        welcomeQMLPage.forceActiveFocus()
    }

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
        }
    }

    Label {
        visible: finishModule.isComplete
        font.weight: Font.Light
        font.pointSize: 32
        text: qsTr("Setup Complete")
        anchors.centerIn: parent
        color: Kirigami.Theme.textColor
    }

    Button {
        id: finishBtn
        width: Kirigami.Units.gridUnit * 4
        height: Kirigami.Units.gridUnit * 3
        focus: true

        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.gridUnit * 6
        }
        flat: true

        Keys.onReturnPressed: clicked()

        onClicked: {
            if (finishModule.isComplete) {
                runningFinishHook = true
                finishModule.runCompleteHook()
            }
        }

        background: Rectangle {
            color: finishModule.isComplete ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.highlightColor
            radius: 2
            border.width: 1
            border.color: finishBtn.activeFocus ? Kirigami.Theme.highlightColor : "transparent"
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

    RowLayout {
        anchors.top: finishBtn.bottom
        anchors.topMargin: Kirigami.Units.gridUnit
        anchors.horizontalCenter: parent.horizontalCenter

        Kirigami.Icon {
            source: finishModule.dir() + "/assets/remote.svg"
            width: Kirigami.Units.iconSizes.smallMedium
            height: Kirigami.Units.iconSizes.smallMedium
        }

        Label {
            id: labelButtonInfo
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.weight: Font.Light
            minimumPointSize: 5
            font.pointSize: 20
            maximumLineCount: 1
            fontSizeMode: Text.Fit
            wrapMode: Text.WordWrap
            text: qsTr("Press Select Button To Continue!")
            color: Kirigami.Theme.textColor
        }
    }
}

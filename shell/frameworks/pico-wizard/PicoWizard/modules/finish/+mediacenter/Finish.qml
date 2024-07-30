// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0


import org.kde.lingmoui 2.7 as LingmoUI

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
            color: LingmoUI.Theme.textColor
        }

        Label {
            visible: finishModule.totalScriptsCount > 0
            Layout.topMargin: 48
            Layout.alignment: Qt.AlignHCenter
            font.weight: Font.Light
            font.pointSize: 10
            text: qsTr("Running Script [%1/%2]").arg(finishModule.runningScriptIndex + 1).arg(finishModule.totalScriptsCount)
            color: LingmoUI.Theme.textColor
        }
    }

    Label {
        visible: finishModule.isComplete
        font.weight: Font.Light
        font.pointSize: 32
        text: qsTr("Setup Complete")
        anchors.centerIn: parent
        color: LingmoUI.Theme.textColor
    }

    Button {
        id: finishBtn
        width: LingmoUI.Units.gridUnit * 4
        height: LingmoUI.Units.gridUnit * 3
        focus: true

        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: LingmoUI.Units.gridUnit * 6
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
            color: finishModule.isComplete ? LingmoUI.Theme.positiveTextColor : LingmoUI.Theme.highlightColor
            radius: 2
            border.width: 1
            border.color: finishBtn.activeFocus ? LingmoUI.Theme.highlightColor : "transparent"
        }

        LingmoUI.Icon {
            visible: finishModule.isComplete && !runningFinishHook
            width: LingmoUI.Units.iconSizes.smallMedium
            height: LingmoUI.Units.iconSizes.smallMedium
            color: LingmoUI.Theme.highlightedTextColor
            isMask: true

            anchors.centerIn: parent
            source: "answer-correct"
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: !finishModule.isComplete || runningFinishHook
            width: LingmoUI.Units.iconSizes.smallMedium
            height: LingmoUI.Units.iconSizes.smallMedium
            Layout.alignment: Qt.AlignHCenter
        }
    }

    RowLayout {
        anchors.top: finishBtn.bottom
        anchors.topMargin: LingmoUI.Units.gridUnit
        anchors.horizontalCenter: parent.horizontalCenter

        LingmoUI.Icon {
            source: finishModule.dir() + "/assets/remote.svg"
            width: LingmoUI.Units.iconSizes.smallMedium
            height: LingmoUI.Units.iconSizes.smallMedium
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
            color: LingmoUI.Theme.textColor
        }
    }
}

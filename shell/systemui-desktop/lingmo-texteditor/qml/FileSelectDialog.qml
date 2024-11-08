/*
 * Copyright (C) 2023 LingmoOS Team.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import LingmoUI 1.0 as LingmoUI

LingmoUI.Window {
    id: control

    property var contentWidth: _mainLayout.implicitWidth + LingmoUI.Units.largeSpacing * 2
    property var contentHeight: _mainLayout.implicitHeight + header.height + LingmoUI.Units.largeSpacing * 2
    property var fileUrl: ""
    property var displayText: ""

    flags: Qt.WindowStaysOnTopHint | Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.WindowModal
    visible: false

    width: contentWidth
    height: contentHeight
    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight
    minimizeButtonVisible: false

    background.color: LingmoUI.Theme.secondBackgroundColor

    signal okBtnClicked

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        spacing: LingmoUI.Units.smallSpacing
        anchors.margins: LingmoUI.Units.smallSpacing

        Label {
            text: displayText
        }
        ColumnLayout {
            spacing: LingmoUI.Units.largeSpacing

            TextInput{
                id:textInput
                height: LingmoUI.Units.largeSpacing
                width: parent.width
                activeFocusOnPress: true
                autoScroll: true
                selectByMouse: true
                horizontalAlignment: TextInput.AlignLeft
                text: fileUrl

                onAccepted: {
                    fileUrl = text
                    control.visible = false
                    control.okBtnClicked()
                }
            }

            RowLayout {
                spacing: LingmoUI.Units.largeSpacing

                Button {
                    text: qsTr("Yes")
                    Layout.fillWidth: true
                    flat: true

                    onClicked: {
                        fileUrl = textInput.text
                        control.visible = false
                        control.okBtnClicked()
                    }
                }

                Button {
                    text: qsTr("Cancel")
                    Layout.fillWidth: true
                    onClicked: control.visible = false
                }
            }
        }
    }
}

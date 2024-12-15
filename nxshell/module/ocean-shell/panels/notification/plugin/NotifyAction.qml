// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0
import org.lingmo.dtk.private 1.0 as DP
import org.lingmo.ds.notification

Control {
    id: root

    property var actions: []
    signal actionInvoked(var actionId)

    contentItem: RowLayout {
        spacing: 5
        height: 30
        NotifyActionButton {
            actionData: actions[0]
            Layout.maximumWidth: 120
            Layout.preferredHeight: 30
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
        }

        Loader {
            active: actions.length === 2
            visible: active
            Layout.maximumWidth: 120
            Layout.preferredHeight: 30
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            sourceComponent: NotifyActionButton {
                actionData: actions[1]
            }
        }

        Loader {
            active: actions.length > 2
            visible: active
            Layout.maximumWidth: 120
            Layout.alignment: Qt.AlignHCenter
            sourceComponent: ComboBox {
                property var expandActions: actions.slice(1)
                textRole: "text"
                padding: 0
                implicitHeight: 30
                implicitWidth: 60
                model: expandActions
                delegate: NotifyActionButton {
                    required property int index

                    actionData: expandActions[index]
                }
            }
        }
    }

    component NotifyActionButton: Button {
        id: actionButton
        required property var actionData
        text: actionData.text
        topPadding: undefined
        bottomPadding: undefined
        leftPadding: undefined
        rightPadding: undefined
        padding: 0
        spacing: 0
        font: DTK.fontManager.t6
        onClicked: {
            console.log("action invoked", actionData.id)
            actionInvoked(actionData.id)
        }

        Loader {
            anchors.fill: parent
            active: actionButton.activeFocus

            sourceComponent: FocusBoxBorder {
                radius: 6
                color: palette.highlight
            }
        }

        background: NotifyItemBackground {
            implicitHeight: 30
            implicitWidth: 50
            radius: 6
            outsideBorderColor: null
            insideBorderColor: null
            anchors.fill: parent
        }
    }
}

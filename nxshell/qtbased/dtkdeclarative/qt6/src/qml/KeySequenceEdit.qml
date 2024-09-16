// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private 1.0 as P

Control {
    id: control
    property string text
    property string placeholderText
    property alias keys: listener.keys
    property D.Palette backgroundColor: DS.Style.keySequenceEdit.background
    property D.Palette placeholderTextColor: DS.Style.keySequenceEdit.placeholderText

    background: Rectangle {
        implicitWidth: DS.Style.keySequenceEdit.width
        implicitHeight: DS.Style.keySequenceEdit.height
        radius: DS.Style.control.radius
        color: control.D.ColorSelector.backgroundColor
    }

    contentItem: RowLayout {
        Label {
            Layout.leftMargin: DS.Style.keySequenceEdit.margin
            text: control.text
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }

        Component {
            id: inputComponent
            Label {
                text: control.placeholderText
                color: control.D.ColorSelector.placeholderTextColor
                font: D.DTK.fontManager.t7
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
            }
        }

        Component {
            id: keyComponent
            RowLayout {
                spacing: DS.Style.keySequenceEdit.margin
                Repeater {
                    model: control.keys
                    P.KeySequenceLabel {
                        Layout.alignment: Qt.AlignRight
                        text: modelData
                    }
                }
            }
        }

        Loader {
            Layout.rightMargin: DS.Style.keySequenceEdit.margin
            Layout.alignment: Qt.AlignVCenter
            sourceComponent: (control.keys.length !== 0) ? keyComponent : inputComponent

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    control.forceActiveFocus()
                    listener.clearKeys()
                }
            }
        }
    }

    D.KeySequenceListener {
        id: listener
        target: control
    }
}

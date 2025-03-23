// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

LineEdit {
    id: control
    property alias placeholder: centerIndicatorLabel.text
    property bool editting: control.activeFocus || (text.length !== 0)
    leftPadding: (editting) ? searchIcon.width + DS.Style.searchEdit.iconLeftMargin
                              + DS.Style.searchEdit.iconRightMargin
                            : 0

    // The search background will be hidden in the focus state,
    // no text input and displayed in the non focus state.
    Item {
        id: searchBackground
        anchors.fill: parent

        RowLayout {
            id: centerIndicator
            spacing: DS.Style.control.spacing

            // Search Icon
            D.DciIcon {
                id: searchIcon
                name: "action_search"
                sourceSize: Qt.size(DS.Style.searchEdit.iconSize, DS.Style.searchEdit.iconSize)
                palette: D.DTK.makeIconPalette(control.palette)
                mode: control.D.ColorSelector.controlState
                theme: control.D.ColorSelector.controlTheme
                fallbackToQIcon: false
            }

            Text {
                id: centerIndicatorLabel
                text: qsTr("Search")
                font: control.font
                verticalAlignment: Text.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            visible: !editting
            onClicked: function(mouse) {
                control.forceActiveFocus(Qt.MouseFocusReason)
                mouse.accepted = false
            }
        }

        state: "NONEDIT"
        states: [
            State {
                name: "EDITTING"
                when: editting
                AnchorChanges {
                    target: centerIndicator
                    anchors.left: searchBackground.left
                    anchors.verticalCenter: searchBackground.verticalCenter
                }
                PropertyChanges {
                    target: centerIndicator
                    anchors.leftMargin: DS.Style.searchEdit.iconLeftMargin
                }
                PropertyChanges {
                    target: centerIndicatorLabel
                    color: "transparent"
                }
            },
            State {
                name: "NONEDIT"
                when: !editting
                AnchorChanges {
                    target: centerIndicator
                    anchors.horizontalCenter: searchBackground.horizontalCenter
                    anchors.verticalCenter: searchBackground.verticalCenter
                }
                PropertyChanges {
                    target: centerIndicatorLabel
                    color: control.placeholderTextColor
                }
            }
        ]

        transitions: Transition {
            AnchorAnimation {
                duration: DS.Style.searchEdit.animationDuration
                easing.type: Easing.OutCubic
            }
            ColorAnimation {
                duration: DS.Style.searchEdit.animationDuration
                easing.type: Easing.OutCubic
            }
        }
    }
}

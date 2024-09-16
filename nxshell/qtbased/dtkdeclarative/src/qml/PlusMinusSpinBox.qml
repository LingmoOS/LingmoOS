// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk.style 1.0 as DS

FocusScope {
    id: control
    // Default SpinBox Properties
    property alias spinBox: spinBoxControl
    // Button Visible Properties
    property alias upButtonVisible: plusButton.active
    property alias downButtonVisible: minusButton.active
    property alias resetButtonVisible: resetButton.active

    implicitWidth: DS.Style.spinBox.width
    implicitHeight: DS.Style.spinBox.height

    RowLayout {
        anchors.fill: parent
        spacing: DS.Style.spinBox.spacing
        SpinBox {
            id: spinBoxControl
            Layout.fillWidth: true
            up.indicator: null
            down.indicator: null
        }

        Loader {
            id: plusButton
            active: true
            sourceComponent: IconButton {
                implicitWidth: DS.Style.spinBox.height
                implicitHeight: DS.Style.spinBox.height
                focusPolicy: Qt.NoFocus
                icon.name: "action_add"
                icon.width: 16
                onClicked: spinBoxControl.increase()
            }
        }

        Loader {
            id: minusButton
            active: true
            sourceComponent: IconButton {
                implicitWidth: DS.Style.spinBox.height
                implicitHeight: DS.Style.spinBox.height
                focusPolicy: Qt.NoFocus
                icon.name: "action_reduce"
                icon.width: 16
                onClicked: spinBoxControl.decrease()
            }
        }

        Loader {
            id: resetButton
            active: false
            sourceComponent: IconButton {
                implicitWidth: DS.Style.spinBox.height
                implicitHeight: DS.Style.spinBox.height
                focusPolicy: Qt.NoFocus
                icon.name: "action_reset"
                icon.width: 16
                onClicked: { spinBoxControl.value = 0 }
            }
        }
    }
}

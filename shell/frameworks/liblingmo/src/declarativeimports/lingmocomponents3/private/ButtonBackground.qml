/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

Item {
    id: root

    required property T.Button button

    // These should be used as the padding for the parent control
    property real topMargin: loader.item.topMargin
    property real leftMargin: loader.item.leftMargin
    property real rightMargin: loader.item.rightMargin
    property real bottomMargin: loader.item.bottomMargin

    implicitWidth: LingmoUI.Units.gridUnit + root.leftMargin + root.rightMargin
    implicitHeight: LingmoUI.Units.gridUnit + root.topMargin + root.bottomMargin

    opacity: enabled ? 1 : 0.5
    layer.enabled: opacity < 1

    Loader {
        id: loader
        anchors.fill: parent
        sourceComponent: root.button.flat ? flatButtonBackground : raisedButtonBackground
    }

    Component {
        id: flatButtonBackground
        FlatButtonBackground {
            anchors.fill: parent
            hovered: root.button.hovered
            pressed: root.button.down
            checked: root.button.checked
            focused: root.button.visualFocus
        }
    }

    Component {
        id: raisedButtonBackground
        RaisedButtonBackground {
            anchors.fill: parent
            hovered: root.button.hovered
            pressed: root.button.down
            checked: root.button.checked
            focused: root.button.visualFocus
        }
    }
}

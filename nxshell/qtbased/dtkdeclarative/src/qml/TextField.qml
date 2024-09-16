// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.TextField {
    id: control
    property D.Palette placeholderTextColor: DS.Style.edit.placeholderText
    property alias backgroundColor: panel.backgroundColor
    // alert control properties
    property alias alertText: panel.alertText
    property alias alertDuration: panel.alertDuration
    property alias showAlert: panel.showAlert

    implicitWidth: Math.max(DS.Style.control.implicitWidth(control),
                            placeholderText ? placeholder.implicitWidth + leftPadding + rightPadding
                                            : 0) || contentWidth + leftPadding + rightPadding
    implicitHeight: Math.max(DS.Style.control.implicitHeight(control),
                             placeholder.implicitHeight + topPadding + bottomPadding)
    padding: DS.Style.control.padding
    color: control.palette.text
    opacity: enabled ? 1 : 0.4
    selectionColor: control.palette.highlight
    selectedTextColor: control.palette.highlightedText
    verticalAlignment: TextInput.AlignVCenter
    onEffectiveHorizontalAlignmentChanged: placeholder.effectiveHorizontalAlignmentChanged()

    Loader {
        id: placeholder
        active: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)
        signal effectiveHorizontalAlignmentChanged

        sourceComponent: PlaceholderText {
            text: control.placeholderText
            font: control.font
            color: control.D.ColorSelector.placeholderTextColor
            verticalAlignment: control.verticalAlignment
            renderType: control.renderType
        }
    }

    background: EditPanel {
        id: panel
        control: control
        implicitWidth: DS.Style.edit.width
        implicitHeight: DS.Style.edit.textFieldHeight
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onClicked: function (mouse) {
            var point = Qt.point(mouse.x, mouse.y)
            if (Window.window && Window.window.width > Screen.desktopAvailableWidth) {
                point.x = control.mapFromGlobal(Screen.desktopAvailableWidth - contextMenu.width).x
            }
            if (Window.window && Window.window.height > Screen.desktopAvailableHeight) {
                point.y = control.mapFromGlobal(0, Screen.desktopAvailableHeight - contextMenu.height).y
            }
            contextMenu.popup(point.x, point.y)
        }
    }

    Menu {
        id: contextMenu

        MenuItem
        {
            text: qsTr("Copy")
            onTriggered: control.copy()
            enabled: control.selectedText.length
        }

        MenuItem
        {
            text: qsTr("Cut")
            onTriggered: control.cut()
            enabled: control.selectedText.length
        }

        MenuItem
        {
            text: qsTr("Paste")
            onTriggered: control.paste()
            enabled: control.canPaste
        }

        MenuItem
        {
            text: qsTr("Select All")
            onTriggered: control.selectAll()
            enabled: control.text.length
        }

        MenuItem
        {
            text: qsTr("Undo")
            onTriggered: control.undo()
            enabled: control.canUndo
        }

        MenuItem
        {
            text: qsTr("Redo")
            onTriggered: control.redo()
            enabled: control.canRedo
        }
    }

}

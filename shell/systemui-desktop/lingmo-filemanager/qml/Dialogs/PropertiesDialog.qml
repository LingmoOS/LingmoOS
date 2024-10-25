/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI

Item {
    id: control

    property int widthValue: _mainLayout.implicitWidth + LingmoUI.Units.largeSpacing * 3
    property int heightValue: _mainLayout.implicitHeight + LingmoUI.Units.largeSpacing * 3

    width: widthValue
    height: heightValue

    onWidthValueChanged: main.updateSize(widthValue, heightValue)
    onHeightValueChanged: main.updateSize(widthValue, heightValue)

    focus: true
    Keys.enabled: true
    Keys.onEscapePressed: main.reject()

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.secondBackgroundColor
    }

    onVisibleChanged: {
        if (visible) updateWindowSize()
    }

    function close() {
        main.close()
    }

    function updateWindowSize() {
        if (visible) {
            if (_textField.enabled)
                _textField.forceActiveFocus()
        }
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
        anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
        anchors.topMargin: LingmoUI.Units.smallSpacing
        anchors.bottomMargin: LingmoUI.Units.largeSpacing * 1.5
        spacing: LingmoUI.Units.largeSpacing

        RowLayout {
            spacing: LingmoUI.Units.largeSpacing * 2

            LingmoUI.IconItem {
                width: 64
                height: 64
                source: main.iconName
            }

            TextField {
                id: _textField
                text: main.fileName
                focus: true
                Layout.fillWidth: true
                Keys.onEscapePressed: main.reject()
                enabled: main.isWritable
            }
        }

        GridLayout {
            columns: 2
            columnSpacing: LingmoUI.Units.largeSpacing
            rowSpacing: LingmoUI.Units.largeSpacing
            Layout.alignment: Qt.AlignTop

            onHeightChanged: updateWindowSize()
            onImplicitHeightChanged: updateWindowSize()

            Label {
                text: qsTr("Type:")
                Layout.alignment: Qt.AlignRight
                color: LingmoUI.Theme.disabledTextColor
                visible: mimeType.visible
            }

            Label {
                id: mimeType
                text: main.mimeType
                visible: text
            }

            Label {
                text: qsTr("Location:")
                Layout.alignment: Qt.AlignRight
                color: LingmoUI.Theme.disabledTextColor
            }

            Label {
                id: location
                text: main.location
            }

            Label {
                text: qsTr("Size:")
                Layout.alignment: Qt.AlignRight
                color: LingmoUI.Theme.disabledTextColor
            }

            Label {
                id: size
                text: main.fileSize ? main.fileSize : qsTr("Calculating...")
            }

            Label {
                text: qsTr("Created:")
                Layout.alignment: Qt.AlignRight
                color: LingmoUI.Theme.disabledTextColor
                visible: creationTime.visible
            }

            Label {
                id: creationTime
                text: main.creationTime
                visible: text
            }

            Label {
                text: qsTr("Modified:")
                Layout.alignment: Qt.AlignRight
                color: LingmoUI.Theme.disabledTextColor
                visible: modifiedTime.visible
            }

            Label {
                id: modifiedTime
                text: main.modifiedTime
                visible: text
            }

            Label {
                text: qsTr("Accessed:")
                Layout.alignment: Qt.AlignRight
                color: LingmoUI.Theme.disabledTextColor
                visible: accessTime.visible
            }

            Label {
                id: accessTime
                text: main.accessedTime
                visible: text
            }
        }

        Item {
            height: LingmoUI.Units.smallSpacing
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                onClicked: main.reject()
            }

            Button {
                text: qsTr("OK")
                Layout.fillWidth: true
                onClicked: {
                    main.accept(_textField.text)
                }
                flat: true
            }
        }
    }
}

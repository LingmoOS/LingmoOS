// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
import QtQml.Models 2.11
import org.deepin.dtk 1.0

ColumnLayout {
    id: control
    property string title
    property alias model: model.model
    property alias view: info
    property Component action
    signal clicked(string key)
    property int itemSpacing: 20
    property font itemFont: DTK.fontManager.t5
    property ButtonGroup group: ButtonGroup {}

    width: 220
    spacing: 20
    RowLayout {
        Layout.preferredHeight: 24
        Layout.preferredWidth: 200
        Label {
            text: title
            font: DTK.fontManager.t5
            Layout.fillWidth: true; Layout.preferredHeight: 24
        }
        Loader {
            sourceComponent: control.action
            Layout.alignment: Qt.AlignRight
        }
    }
    ListView {
        id: info
        spacing: control.itemSpacing
        Layout.preferredHeight: contentHeight
        Layout.fillWidth: true
        model: DelegateModel {
            id: model
            delegate: ItemDelegate {
                width: 200
                icon.name: model.icon
                text: model.text
                checked: model.checked
                backgroundVisible: false
                font: control.itemFont
                ButtonGroup.group: group
                onClicked: control.clicked(model.index)
            }
        }
    }
}

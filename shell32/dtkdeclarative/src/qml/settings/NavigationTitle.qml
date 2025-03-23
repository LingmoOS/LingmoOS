// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.settings 1.0 as Settings
import org.deepin.dtk.style 1.0 as DS
import ".."

Control {
    id: control
    signal clicked()
    property bool checked: false
    property D.Palette backgroundColor: DS.Style.settings.background
    property D.Palette checkedTextColor: DS.Style.checkedButton.text

    palette.windowText: checked ? D.ColorSelector.checkedTextColor : undefined
    contentItem: Label {
        text: Settings.SettingsGroup.name
        font: __getFont(Settings.SettingsGroup.level)
        leftPadding: __getMargin(Settings.SettingsGroup.level)
        topPadding: DS.Style.settings.navigation.textVPadding
        bottomPadding: DS.Style.settings.navigation.textVPadding
        anchors.left: parent.left
        verticalAlignment: Qt.AlignVCenter
        elide: Text.ElideRight

        function __getFont(level) {
            switch(level) {
            case 0:
                return D.DTK.fontManager.t4
            // case 1:
            //     return D.DTK.fontManager.t5
            }
            return D.DTK.fontManager.t6
        }
        function __getMargin(level) {
            switch(level) {
            case 0:
                return DS.Style.settings.title.marginL1
            case 1:
                return DS.Style.settings.title.marginL2
            }
            return DS.Style.settings.title.marginLOther
        }
    }

    background: Item {
        implicitHeight: DS.Style.settings.navigation.height
        implicitWidth: DS.Style.settings.navigation.width
        Rectangle {
            anchors.fill: parent
            visible: !control.checked
            color: control.D.ColorSelector.backgroundColor
            radius: DS.Style.control.radius
        }
        HighlightPanel {
            anchors.fill: parent
            visible: control.checked
        }
    }

    MouseArea {
        anchors.fill: parent
        Component.onCompleted: clicked.connect(control.clicked)
    }
}

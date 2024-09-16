// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQml.Models
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS

ColumnLayout {
    default property alias content: itemModel.children
    // 提供隐藏菜单接口
    property alias showProperty: info.visible
    property string title

    spacing: 10
    width: 280

    ItemDelegate {
        id: titleBar

        property Palette titleTextColor: Palette {
            normal: Qt.rgba(0, 0, 0, 1)
            normalDark: Qt.rgba(1, 1, 1, 1)
        }

        Layout.fillWidth: true
        Layout.preferredHeight: 24
        anchors.bottomMargin: 10
        backgroundVisible: false
        checkable: false
        display: IconLabel.IconBesideText
        font: DTK.fontManager.t5
        leftPadding: 10
        palette.windowText: ColorSelector.titleTextColor
        text: title

        icon {
            height: 12
            name: info.visible ? "arrow_ordinary_up" : "arrow_ordinary_down"
            width: 12
        }

        MouseArea {
            anchors.fill: parent

            onClicked: {
                info.visible = !info.visible;
            }
        }
    }

    ListView {
        id: info

        Layout.fillWidth: true
        Layout.preferredHeight: contentHeight
        interactive: false
        spacing: 10

        model: ObjectModel {
            id: itemModel

        }

        Component.onCompleted: {
            for (var i = 0; i < count; ++i) {
                var item = model.get(i);
                item.width = width;
            }
        }
    }
}

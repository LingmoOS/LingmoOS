// Copyright (C) 2022 UnionTech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import org.deepin.dtk 1.0
import "../Control"

ColumnLayout {
    id: control
    property string title
    property ListModel sideModel
    property alias view: sideListView
    property Component action
    signal itemClicked(string uuid, string text)
    signal itemRightClicked(string uuid, string text)
    signal itemCheckedChanged(int index, bool checked)
    signal removeDeviceBtnClicked(string uuid)
    signal sigRemoveItem()
    property ButtonGroup group: ButtonGroup {}
    property bool showTitle : true
    property bool showRemoveDeviceBtn : false

    // 根据uuid获取所在行索引
    function indexFromUuid(uuid) {
        for (var i = 0; i < sideModel.count; i++) {
            if (Number(sideModel.get(i).uuid) === Number(uuid)) {
                return i
            }
        }

        return -1
    }

    RowLayout {
        id: siderTitle
        visible: showTitle
        Layout.preferredWidth: 180; Layout.preferredHeight: 20
        Label {
            id: viewLabel
            width: 42; height: 20
            text: title
            font: DTK.fontManager.t6
            Layout.alignment: Qt.AlignLeft; Layout.leftMargin: 20
            horizontalAlignment: Qt.AlignLeft

            color: Qt.rgba(0.6,0.6,0.6, 0.8)
        }
        Loader {
            Layout.alignment: Qt.AlignRight; Layout.rightMargin: 5
            sourceComponent: control.action
        }
    }
    spacing: 10
    ListView {
        id: sideListView
        implicitHeight: contentHeight
        width: 180
        Layout.alignment: Qt.AlignLeft; Layout.leftMargin: 10
        model: sideModel
        interactive: false
        delegate: SideBarItemDelegate{
            id: sidebarItem
            width: 180
            height: 36
            checked: sideModel.get(index).checked
            backgroundVisible: false
            ButtonGroup.group: group
        }
        Keys.onPressed: (event)=> {
            switch (event.key){
            case Qt.Key_F2:
                sideListView.currentItem.rename();
                break;
            case Qt.Key_Delete:
                if(sideModel.get(currentIndex).deleteable){
                    sigRemoveItem()
                }
                break
            default:
                break;
            }
            event.accepted = true;
        }
    }

}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0
import "../Utils"
import "../Components"

RowLayout {
    anchors.fill: parent
    spacing: Utils.spacing
    ListView {
        id: listView
        clip: true
        spacing: Utils.spacing
        Layout.fillHeight: true
        Layout.preferredWidth: Utils.listItemWidth
        Layout.alignment: Qt.AlignVCenter
        model: pluginListModel
        focus: true
        interactive: true
        highlight: ListViewHighLight {}
        delegate: ListViewDelegate {
            required property int index
            // NOTE: Custom properties need to be kept at the end,
            //       otherwise the index property will not be accessible
            required property string path
            id: delegate
            width: listView.width
            height: Utils.listItemHeight
            isCurrentIndex: listView.currentIndex === index
            onClicked: {
                listView.currentIndex = index
            }
            Loader {
                id: _loader
                visible: false
                source: path + "/main.qml"
                onLoaded: {
                    delegate._icon = path + "/assets/" + _loader.item.icon
                    delegate._title = _loader.item.displayName
                }
            }
        }
        onCurrentItemChanged: {
            view.setSource(listView.currentItem.path + "/main.qml")
        }
    }
    Rectangle {
        id: view
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.bottomMargin: Utils.margin
        Layout.rightMargin: Utils.margin
        color: palette.alternateBase
        radius: Utils.listRadius

        function setSource(source) {
            loader.source = source
        }

        Loader {
            id: loader
            anchors.fill: parent
            asynchronous: true
            visible: status == Loader.Ready
        }
    }
}


// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private 1.0 as P

FocusScope {
    id: control
    property int maxVisibleItems : DS.Style.arrowListView.maxVisibleItems
    property int itemHeight:  DS.Style.arrowListView.itemHeight
    property alias view: itemsView

    implicitWidth: Math.max(DS.Style.arrowListView.width, contentLayout.implicitWidth)
    implicitHeight: contentLayout.implicitHeight

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        P.ArrowListViewButton {
            visible: itemsView.interactive
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: width
            Layout.preferredHeight: height
            view: itemsView
            direction: P.ArrowListViewButton.UpButton
        }

        ListView {
            id: itemsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: Math.min(contentHeight, maxVisibleItems * itemHeight)
            implicitWidth:{
                var maxWidth = DS.Style.arrowListView.width
                if (!itemsView.model || !itemsView.model.hasOwnProperty("get"))
                    return maxWidth

                for (var i = 0; i < itemsView.count; ++i) {
                    var item = itemsView.model.get(i)
                    if (item && item.implicitWidth > maxWidth)
                        maxWidth = item.implicitWidth
                }
                return maxWidth
            }
            interactive: Window.window ? (contentHeight > Window.window.height || model.count > maxVisibleItems) : false
            ScrollIndicator.vertical: ScrollIndicator { }
            highlight: Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 6
                anchors.rightMargin: 6
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                property D.Palette backgroundColor: DS.Style.highlightPanel.background
                color: D.ColorSelector.backgroundColor
                radius: 6 // TODO can't display background when using dtk's InWindowBlur.
            }
            highlightFollowsCurrentItem: true
            highlightMoveDuration: -1
            highlightMoveVelocity: 400
        }

        P.ArrowListViewButton {
            visible: itemsView.interactive
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: width
            Layout.preferredHeight: height
            view: itemsView
            direction: P.ArrowListViewButton.DownButton
        }
    }
}

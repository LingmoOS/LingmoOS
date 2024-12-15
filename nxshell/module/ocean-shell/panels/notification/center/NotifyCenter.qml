// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0
import org.lingmo.ds.notificationcenter

FocusScope {
    id: root

    palette: DTK.palette

    property alias model: notifyModel
    property int maxViewHeight: 400

    NotifyModel {
        id: notifyModel
    }

    Item {
        objectName: "notificationCenter"
        anchors.fill: parent
        NotifyHeader {
            id: header
            anchors {
                top: parent.top
                left: parent.left
            }
            height: 40
            width: parent.width
            notifyModel: notifyModel
            z: 1
        }

        NotifyView {
            id: view
            anchors {
                left: parent.left
                top: header.bottom
                topMargin: 10
                bottom: parent.bottom
            }

            width: parent.width
            height: Math.min(maxViewHeight, viewHeight)
            notifyModel: notifyModel
        }
    }
}

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0
import org.lingmo.ds.notification

Control {
    id: root

    property string appName: "lingmo-editor"
    property string iconName: "lingmo-editor"
    property string content: "content"
    property string title: "title"
    property string date: "date"
    property var actions: [
        {text: "open", id: "open"},
        {text: "close", id: "close"},
        {text: "exec", id: "exec"}
    ]
    property bool strongInteractive: false
    property string contentIcon: "lingmo-editor"
    property int contentRowCount: 6

    signal remove()
    signal setting(var pos)
    signal actionInvoked(var actionId)
    signal linkActivated(var link)

    onLinkActivated: function (link) {
        console.log("Link actived", link)
        ApplicationHelper.openUrl(link)
    }
}

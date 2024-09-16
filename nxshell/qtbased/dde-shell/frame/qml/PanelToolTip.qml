// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.deepin.ds 1.0
import org.deepin.dtk 1.0
import org.deepin.dtk.style as DStyle

Item {
    id: control

    default property alias toolTipContentItem: toolTip.contentItem
    property string text
    property alias toolTipVisible: toolTip.visible
    property var toolTipWindow: Panel.toolTipWindow
    property int toolTipX: 0
    property int toolTipY: 0
    property bool readyBinding: false
    width: toolTip.width
    height: toolTip.height

    Binding {
        when: readyBinding
        target: toolTipWindow; property: "width"
        value: toolTip.width + toolTip.leftPadding + toolTip.rightPadding
    }
    Binding {
        when: readyBinding
        target: toolTipWindow; property: "height"
        value: toolTip.height
    }
    Binding {
        when: readyBinding
        delayed: true
        target: toolTipWindow; property: "xOffset"
        value: control.toolTipX - (toolTip.leftPadding + toolTip.rightPadding) / 2
    }
    Binding {
        when: readyBinding
        delayed: true
        target: toolTipWindow; property: "yOffset"
        value: control.toolTipY
    }

    function open()
    {
        if (!toolTipWindow)
            return

        readyBinding = Qt.binding(function () {
            return toolTipWindow && toolTipWindow.currentItem === control
        })

        toolTipWindow.currentItem = control
        timer.start()
    }

    Timer {
        id: timer
        interval: 10
        onTriggered: {
            if (!toolTipWindow)
                return

            if (!readyBinding)
                return

            toolTipWindow.show()
        }
    }
    
    function close()
    {
        if (!toolTipWindow)
            return

        if (!readyBinding)
            return

        toolTipWindow.close()
        toolTipWindow.currentItem = null
    }
    function hide()
    {
        close()
    }

    Control {
        id: toolTip
        visible: readyBinding
        anchors.centerIn: parent
        parent: toolTipWindow ? toolTipWindow.contentItem : undefined
        font: DTK.fontManager.t6
        contentItem: Text {
            topPadding: 4
            bottomPadding: 4
            leftPadding: 8
            rightPadding: 8

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font: toolTip.font
            wrapMode: Text.WordWrap
            text: control.text
            color: toolTip.palette.brightText
        }
    }
}

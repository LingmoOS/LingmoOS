// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.deepin.ds 1.0

Item {
    id: control
    visible: false
    default property alias popupContent: popup.children
    property alias popupVisible: popup.visible
    property var popupWindow: Panel.popupWindow
    property int popupX: 0
    property int popupY: 0
    property bool readyBinding: false
    width: popup.childrenRect.width
    height: popup.childrenRect.height

    Binding {
        when: readyBinding
        target: popupWindow; property: "width"
        value: popup.width
    }
    Binding {
        when: readyBinding
        target: popupWindow; property: "height"
        value: popup.height
    }
    Binding {
        when: readyBinding
        delayed: true
        target: popupWindow; property: "xOffset"
        value: control.popupX
    }
    Binding {
        when: readyBinding
        delayed: true
        target: popupWindow; property: "yOffset"
        value: control.popupY
    }

    function open()
    {
        if (popup.visible) {
            close()
            return
        }

        if (!popupWindow)
            return

        readyBinding = Qt.binding(function () {
            return popupWindow && popupWindow.currentItem === control
        })

        popupWindow.currentItem = control
        timer.start()
    }

    Timer {
        id: timer
        interval: 10
        onTriggered: {
            if (!popupWindow)
                return

            if (!readyBinding)
                return

            popupWindow.show()
            popupWindow.requestActivate()
        }
    }
    function close()
    {
        if (!popupWindow)
            return

        // avoid to closing window by other PanelPopup.
        if (!readyBinding)
            return

        popupWindow.close()
        popupWindow.currentItem = null
    }

    Connections {
        target: popupWindow
        function onActiveChanged()
        {
            if (!popupWindow)
                return
            // TODO why activeChanged is not emit.
            if (popupWindow && !popupWindow.active) {
                control.close()
            }
        }
    }

    Item {
        id: popup
        visible: readyBinding
        width: control.width
        height: control.height
        parent: popupWindow ? popupWindow.contentItem : undefined
    }
    Component.onDestruction: {
        if (popup.visible)
            control.close()
    }
}

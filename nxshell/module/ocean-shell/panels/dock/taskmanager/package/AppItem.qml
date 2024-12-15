// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.lingmo.ds 1.0
import org.lingmo.ds.dock 1.0
import org.lingmo.dtk 1.0 as D
import Qt.labs.platform 1.1 as LP

Item {
    id: root
    required property int displayMode
    required property int colorTheme
    required property bool active
    required property bool attention
    required property string itemId
    required property string name
    required property string iconName
    required property string menus
    required property list<string> windows
    required property int visualIndex

    signal clickItem(itemId: string, menuId: string)
    signal dragFinished()

    Drag.active: mouseArea.drag.active
    Drag.source: root
    Drag.hotSpot.x: icon.width / 2
    Drag.hotSpot.y: icon.height / 2
    Drag.dragType: Drag.Automatic
    Drag.mimeData: { "text/x-ocean-dock-dnd-appid": itemId, "text/x-ocean-dock-dnd-source": "taskbar" }

    property bool useColumnLayout: Panel.position % 2
    property int statusIndicatorSize: useColumnLayout ? root.width * 0.72 : root.height * 0.72
    property int iconSize: Panel.rootObject.dockItemMaxSize * 9 / 14

    property var iconGlobalPoint: {
        var a = icon
        var x = 0, y = 0
        while(a.parent) {
            x += a.x
            y += a.y
            a = a.parent
        }

        return Qt.point(x, y)
    }

    Item {
        anchors.fill: parent
        id: appItem
        visible: !root.Drag.active // When in dragging, hide app item
        AppItemPalette {
            id: itemPalette
            displayMode: root.displayMode
            colorTheme: root.colorTheme
            active: root.active
            backgroundColor: D.DTK.palette.highlight
        }

        StatusIndicator {
            id: statusIndicator
            palette: itemPalette
            width: root.statusIndicatorSize
            height: root.statusIndicatorSize
            anchors.centerIn: icon
            visible: root.displayMode === Dock.Efficient && root.windows.length > 0
        }

        WindowIndicator {
            id: windowIndicator
            dotWidth: root.useColumnLayout  ? Math.max(iconSize / 16, 2) : Math.max(iconSize / 3, 2)
            dotHeight: root.useColumnLayout ? Math.max(iconSize / 3, 2) : Math.max(iconSize / 16, 2)
            windows: root.windows
            displayMode: root.displayMode
            useColumnLayout: root.useColumnLayout
            palette: itemPalette
            visible: (root.displayMode === Dock.Efficient && root.windows.length > 1) || (root.displayMode === Dock.Fashion && root.windows.length > 0)

            function updateIndicatorAnchors() {
                windowIndicator.anchors.top = undefined
                windowIndicator.anchors.topMargin = 0
                windowIndicator.anchors.bottom = undefined
                windowIndicator.anchors.bottomMargin = 0
                windowIndicator.anchors.left = undefined
                windowIndicator.anchors.leftMargin = 0
                windowIndicator.anchors.right = undefined
                windowIndicator.anchors.rightMargin = 0
                windowIndicator.anchors.horizontalCenter = undefined
                windowIndicator.anchors.verticalCenter = undefined

                switch(Panel.position) {
                case Dock.Top: {
                    windowIndicator.anchors.horizontalCenter = parent.horizontalCenter
                    windowIndicator.anchors.top = parent.top
                    windowIndicator.anchors.topMargin = Qt.binding(() => {return (root.height - iconSize) / 2 / 3})
                    return
                }
                case Dock.Bottom: {
                    windowIndicator.anchors.horizontalCenter = parent.horizontalCenter
                    windowIndicator.anchors.bottom = parent.bottom
                    windowIndicator.anchors.bottomMargin = Qt.binding(() => {return (root.height - iconSize) / 2 / 3})
                    return
                }
                case Dock.Left: {
                    windowIndicator.anchors.verticalCenter = parent.verticalCenter
                    windowIndicator.anchors.left = parent.left
                    windowIndicator.anchors.leftMargin = Qt.binding(() => {return (root.width - iconSize) / 2 / 3})
                    return
                }
                case Dock.Right:{
                    windowIndicator.anchors.verticalCenter = parent.verticalCenter
                    windowIndicator.anchors.right = parent.right
                    windowIndicator.anchors.rightMargin = Qt.binding(() => {return (root.width - iconSize) / 2 / 3})
                    return
                }
                }
            }

            Component.onCompleted: {
                windowIndicator.updateIndicatorAnchors()
            }
        }

        Connections {
            function onPositionChanged() {
                windowIndicator.updateIndicatorAnchors()
                updateWindowIconGeometryTimer.start()
            }
            target: Panel
        }

        Loader {
            id: contextMenuLoader
            active: false
            sourceComponent: LP.Menu {
                id: contextMenu
                Instantiator {
                    id: menuItemInstantiator
                    model: JSON.parse(menus)
                    delegate: LP.MenuItem {
                        text: modelData.name
                        onTriggered: {
                            root.clickItem(root.itemId, modelData.id)
                        }
                    }
                    onObjectAoceand: (index, object) => contextMenu.insertItem(index, object)
                    onObjectRemoved: (index, object) => contextMenu.removeItem(object)
                }
            }
        }

        D.DciIcon {
            id: icon
            name: root.iconName
            height: iconSize
            width: iconSize
            sourceSize: Qt.size(iconSize, iconSize)
            anchors.centerIn: parent
            retainWhileLoading: true
            scale: Panel.rootObject.isDragging ? 1.0 : 1.0

            LaunchAnimation {
                id: launchAnimation
                launchSpace: {
                    switch (Panel.position) {
                    case Dock.Top:
                    case Dock.Bottom:
                    return (root.height - icon.height) / 2
                    case Dock.Left:
                    case Dock.Right:
                        return (root.width - icon.width) / 2
                    }
                }

                direction: {
                    switch (Panel.position) {
                    case Dock.Top:
                        return LaunchAnimation.Direction.Down
                    case Dock.Bottom:
                        return LaunchAnimation.Direction.Up
                    case Dock.Left:
                        return LaunchAnimation.Direction.Right
                    case Dock.Right:
                        return LaunchAnimation.Direction.Left
                    }
                }
                target: icon
                loops: 1
                running: false
            }
        }

        // TODO: value can set during debugPanel
        Loader {
            anchors.fill: icon
            z: -1
            active: root.attention && !Panel.rootObject.isDragging
            sourceComponent: Repeater {
                model: 5
                Rectangle {
                    id: rect
                    required property int index
                    property var originSize: iconSize

                    width: originSize * (index - 1)
                    height: width
                    radius: width / 2
                    color: Qt.rgba(1, 1, 1, 0.1)

                    anchors.centerIn: parent
                    opacity: Math.min(3 - width / originSize, blendColorAlpha(D.DTK.themeType === D.ApplicationHelper.DarkType ? 0.25 : 1.0))

                    function blendColorAlpha(fallback) {
                        var appearance = DS.applet("org.lingmo.ds.ocean-appearance")
                        if (!appearance || appearance.opacity < 0 || appearance.opacity > fallback || appearance.opacity < fallback)
                            return fallback
                        return appearance.opacity
                    }

                    SequentialAnimation {
                        running: true
                        loops: Animation.Infinite

                        // 弹出
                        ParallelAnimation {
                            NumberAnimation { target: rect; property: "width"; from: Math.max(originSize * (index - 1), 0); to: originSize * (index); duration: 1200 }
                            ColorAnimation { target: rect; property: "color"; from: Qt.rgba(1, 1, 1, 0.4); to: Qt.rgba(1, 1, 1, 0.1); duration: 1200 }
                            NumberAnimation { target: icon; property: "scale"; from: 1.0; to: 1.15; duration: 1200; easing.type: Easing.OutElastic; easing.amplitude: 1; easing.period: 0.2 }
                        }

                        // 收缩
                        ParallelAnimation {
                            NumberAnimation { target: rect; property: "width"; from: originSize * (index); to: originSize * (index + 1); duration: 1200 }
                            ColorAnimation { target: rect; property: "color"; from: Qt.rgba(1, 1, 1, 0.4); to: Qt.rgba(1, 1, 1, 0.1); duration: 1200 }
                            NumberAnimation { target: icon; property: "scale"; from: 1.15; to: 1.0; duration: 1200; easing.type: Easing.OutElastic; easing.amplitude: 1; easing.period: 0.2 }
                        }

                        // 停顿
                        ParallelAnimation {
                            NumberAnimation { target: rect; property: "width"; from: originSize * (index + 1); to: originSize * (index + 2); duration: 1200 }
                            ColorAnimation { target: rect; property: "color"; from: Qt.rgba(1, 1, 1, 0.4); to: Qt.rgba(1, 1, 1, 0.1); duration: 1200 }
                        }
                    }

                    D.BoxShadow {
                        visible: rect.visible
                        anchors.fill: rect
                        z: -2
                        shadowBlur: 20
                        shadowColor : Qt.rgba(0, 0, 0, 0.05)
                        shadowOffsetX : 0
                        shadowOffsetY : 0
                        cornerRadius: rect.radius
                        hollow: true
                    }
                }
            }
        }
    }

    Timer {
        id: updateWindowIconGeometryTimer
        interval: 500
        running: false
        repeat: false
        onTriggered: {
            var pos = icon.mapToItem(null, 0, 0)
            taskmanager.Applet.setAppItemWindowIconGeometry(root.itemId, Panel.rootObject, pos.x, pos.y,
                pos.x + icon.width, pos.y + icon.height)
        }
    }

    Timer {
        id: previewTimer
        interval: 500
        running: false
        repeat: false
        property int xOffset: 0
        property int yOffset: 0
        onTriggered: {
            if (root.windows.length != 0 || Qt.platform.pluginName === "wayland") {
                taskmanager.Applet.showItemPreview(root.itemId, Panel.rootObject, xOffset, yOffset, Panel.position)
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        drag.target: root
        drag.onActiveChanged: {
            if (!drag.active)
                root.dragFinished()
        }

        onPressed: function (mouse) {
            if (mouse.button === Qt.LeftButton) {
                icon.grabToImage(function(result) {
                    root.Drag.imageSource = result.url;
                })
            }
            toolTip.close()
            closeItemPreview()
        }
        onClicked: function (mouse) {
            if (mouse.button === Qt.RightButton) {
                contextMenuLoader.active = true
                MenuHelper.openMenu(contextMenuLoader.item)
            } else {
                if (root.windows.length === 0) {
                    launchAnimation.start()
                }
                root.clickItem(root.itemId, "")
            }
        }

        onEntered: {
            if (Qt.platform.pluginName === "xcb" && windows.length === 0) {
                toolTipShowTimer.start()
                return
            }

            var itemPos = root.mapToItem(null, 0, 0)
            let xOffset, yOffset, interval = 10
            if (Panel.position % 2 === 0) {
                xOffset = itemPos.x + (root.width / 2)
                yOffset = (Panel.position == 2 ? -interval : interval + Panel.dockSize)
            } else {
                xOffset = (Panel.position == 1 ? -interval : interval + Panel.dockSize)
                yOffset = itemPos.y + (root.height / 2)
            }
            previewTimer.xOffset = xOffset
            previewTimer.yOffset = yOffset
            previewTimer.start()
        }

        onExited: {
            if (toolTipShowTimer.running) {
                toolTipShowTimer.stop()
            }

            if (previewTimer.running) {
                previewTimer.stop()
            }

            if (Qt.platform.pluginName === "xcb" && windows.length === 0) {
                toolTip.close()
                return
            }
            closeItemPreview()
        }

        PanelToolTip {
            id: toolTip
            text: root.name
            toolTipX: DockPanelPositioner.x
            toolTipY: DockPanelPositioner.y
        }

        Timer {
            id: toolTipShowTimer
            interval: 50
            onTriggered: {
                var point = root.mapToItem(null, root.width / 2, root.height / 2)
                toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                toolTip.open()
            }
        }

        function closeItemPreview() {
            if (previewTimer.running) {
                previewTimer.stop()
            } else {
                taskmanager.Applet.hideItemPreview()
            }
        }
    }

    onWindowsChanged: {
        updateWindowIconGeometryTimer.start()
    }

    onIconGlobalPointChanged: {
        updateWindowIconGeometryTimer.start()
    }
}

// Copyright (C) 2023 JiDe Zhang <zccrs@live.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import Qt5Compat.GraphicalEffects
import QtQuick.Controls
import QtQuick.Effects
import Waylib.Server
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS
import TreeLand.Utils

D.RoundRectangle {
    id: root
    corners: D.RoundRectangle.TopLeftCorner | D.RoundRectangle.TopRightCorner
    color: "transparent"

    signal requestMove
    signal requestMinimize
    signal requestMaximize(var max)
    signal requestClose
    signal requestResize(var edges)

    readonly property real topMargin: titlebar.height
    readonly property real bottomMargin: 0
    readonly property real leftMargin: 0
    readonly property real rightMargin: 0
    property bool moveable: true
    property bool enable: true
    property D.Palette backgroundColor: DS.Style.highlightPanel.background
    property D.Palette outerShadowColor: DS.Style.highlightPanel.dropShadow
    property D.Palette innerShadowColor: DS.Style.highlightPanel.innerShadow
    required property ToplevelSurface surface

    visible: enable

    D.BoxShadow {
        anchors.fill: root
        shadowColor: root.D.ColorSelector.outerShadowColor
        shadowOffsetY: 4
        shadowBlur: 16
        cornerRadius: root.radius
        hollow: true
    }

    MouseArea {
        property int edges: 0

        anchors {
            fill: parent
            margins: -10
        }

        hoverEnabled: true
        Cursor.shape: {
            switch(edges) {
            case Qt.TopEdge:
                return Cursor.TopSide
            case Qt.RightEdge:
                return Cursor.RightSide
            case Qt.BottomEdge:
                return Cursor.BottomSide
            case Qt.LeftEdge:
                return Cursor.LeftSide
            case Qt.TopEdge | Qt.LeftEdge:
                return Cursor.TopLeftCorner
            case Qt.TopEdge | Qt.RightEdge:
                return Cursor.TopRightCorner
            case Qt.BottomEdge | Qt.LeftEdge:
                return Cursor.BottomLeftCorner
            case Qt.BottomEdge | Qt.RightEdge:
                return Cursor.BottomRightCorner
            }

            return Qt.ArrowCursor;
        }

        onPositionChanged: function (event) {
            edges = WaylibHelper.getEdges(Qt.rect(0, 0, width, height), Qt.point(event.x, event.y), 10)
        }

        onPressed: function (event) {
            // Maybe missing onPositionChanged when use touchscreen
            edges = WaylibHelper.getEdges(Qt.rect(0, 0, width, height), Qt.point(event.x, event.y), 10)
            Helper.activatedSurface = surface
            if(edges)
                root.requestResize(edges)
        }
    }

    WindowMenu {
        id: menu

        onRequestClose: root.requestClose()
        onRequestMaximize: root.requestMaximize(max)
        onRequestMinimize: root.requestMinimize()
        onRequestMove:root.requestMove()
        onRequestResize: root.requestResize(edges)
    }

    D.RoundRectangle {
        id: titlebar
        anchors.top: parent.top
        width: parent.width
        height: 30
        radius: parent.radius
        corners: parent.corners
        layer.enabled: true
    }
    // TODO: use twice element, need optimize.
    /*  CAUTION: impl of mask clip is riscky
    *   layer.effect creates a sibling node and set hideSource to true,
    *   results in both sibling and origin invisible in shadereffectsource when they're not effectiveVisible
    *   (maybe a bug involving hideSource, if set only one shadereffectsource can display)
    *   so use opacity=0 & explicit mask effect item
    */
    Rectangle {
        id: titlebarContent
        anchors.fill: titlebar

        layer.enabled: true
        opacity: 0

        HoverHandler {
            // block hover events to resizing mouse area, avoid cursor change
            cursorShape: Qt.ArrowCursor
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onTapped: (eventPoint, button) => {
                if (button === Qt.RightButton) {
                    menu.popup(eventPoint.position.x, eventPoint.position.y)
                }
                else {
                    Helper.activatedSurface = surface
                }
            }
            onDoubleTapped: (_, button) => {
                if (button === Qt.LeftButton) {
                    root.requestMaximize(!surface.isMaximized)
                }
            }
        }

        DragHandler {
            enabled: moveable
            target: root.parent
            onActiveChanged: if (active) {
                Helper.activatedSurface = surface
                Helper.movingItem = root.parent
            } else {
                Helper.movingItem = null
            }
            cursorShape: active ? Qt.DragMoveCursor : Qt.ArrowCursor
        }

        Label {
            anchors.centerIn: parent
            clip: true
            text: surface.title
        }
        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
            }

            Item {
                id: control
                property D.Palette textColor: DS.Style.button.text
                property D.Palette backgroundColor: DS.Style.windowButton.background
            }

            Loader {
                objectName: "minimizeBtn"
                sourceComponent: D.WindowButton {
                    icon.name: "window_minimize"
                    textColor: control.textColor
                    height: titlebar.height

                    onClicked: {
                        root.requestMinimize()
                    }
                }
            }

            Loader {
                objectName: "quitFullBtn"
                visible: false
                sourceComponent: D.WindowButton {
                    icon.name: "window_quit_full"
                    textColor: control.textColor
                    height: titlebar.height

                    onClicked: {
                    }
                }
            }

            Loader {
                id: maxOrWindedBtn
                objectName: "maxOrWindedBtn"
                sourceComponent: D.WindowButton {
                    icon.name: surface.isMaximized ? "window_restore" : "window_maximize"
                    textColor: control.textColor
                    height: titlebar.height

                    onClicked: {
                        root.requestMaximize(!surface.isMaximized)
                    }
                }
            }

            Loader {
                objectName: "closeBtn"
                sourceComponent: D.WindowButton {
                    icon.name: "window_close"
                    textColor: control.textColor
                    height: titlebar.height

                    onClicked: {
                        root.requestClose()
                    }
                }
            }
        }
    }

    MultiEffect {
        source: titlebarContent
        anchors.fill: titlebar
        maskEnabled: true
        maskSource: titlebar
    }
}

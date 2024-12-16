// Copyright (C) 2023 Dingyuan Zhang <zhangdingyuan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import Waylib.Server
import TreeLand

Item {
    id: root
    required property ListModel model
    property var current: 0
    required property OutputDelegate activeOutput

    signal surfaceActivated(surface: SurfaceWrapper)

    onVisibleChanged: {
        if (visible) {
            model = workspaceManager.workspacesById.get(workspaceManager.layoutOrder.get(currentWorkspaceId).wsid).surfaces
            current = 0
            indicatorPlane.calcLayout()
            show()
        } else {
            stop()
        }
    }

    function previous() {
        current = current - 1
        if (current < 0) {
            current = Math.max(0, model.count - 1)
        }
    }
    function next() {
        current = current + 1
        if (current >= model.count) {
            current = 0
        }
    }

    function show() {
        // current may be in intermediate state, make sure current is in range
        if (current >= model.count || current < 0) {
            return
        }

        const source = model.get(current).item

        context.parent = parent
        context.anchors.fill = root
        context.sourceComponent = contextComponent
        context.item.start(source)
    }

    function stop() {
        if (context.item) {
            context.item.stop()
        }
        const wrapper = model.get(current).wrapper
        // activated window changed
        if (current != 0) {
            surfaceActivated(wrapper)
        }
    }
    
    onCurrentChanged: {
        // change may be triggered by: click, keyboard navigation
        show()
    }

    Loader {
        id: context
    }

    Component {
        id: contextComponent

        WindowsSwitcherPreview {
        }
    }

    Connections {
        target: model
        function onCountChanged() {
            if (root.visible) {
                // in case a window exited when previewing
                current = Math.max(Math.min(current, model.count - 1), 0)
                indicatorPlane.calcLayout()
            }
        }
    }

    property int spacing: 10
    property var rows: []
    property int rowHeight: 0
    property int padding: 8

    Item {
        id: indicatorPlane
        
        // currently use binding, so indicatorPlane follows mouse/active output
        x: {
            const coord=parent.mapFromItem(activeOutput, 0, 0)
            return coord.x
        }
        y: {
            const coord=parent.mapFromItem(activeOutput, 0, 0)
            return coord.y
        }
        width: activeOutput?.width
        height: activeOutput?.height
        
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.visible = false
            }
        }

        Rectangle {
            width: flickable.width + 2 * padding
            height: flickable.height + 2 * padding
            anchors.centerIn: flickable
            radius: 10
            opacity: 0.4
        }

        Flickable {
            id: flickable
            width: eqhgrid.width
            height: Math.min(eqhgrid.height, parent.height * 0.7)
            anchors.centerIn: parent
            contentHeight: eqhgrid.height
            boundsBehavior: Flickable.StopAtBounds
            Behavior on contentY {
                NumberAnimation {
                    duration: 200
                }
            }
            clip: true

            EQHGrid {
                id: eqhgrid
                model: root.model
                minH: 100
                maxH: 200
                maxW: indicatorPlane.width * maxH / indicatorPlane.height
                availW: indicatorPlane.width * .7
                availH: indicatorPlane.height * .7
                itemVerticalPadding: 24 + 2 * padding
                getRatio: (data) => data.item.width / data.item.height
                anchors.centerIn: parent
                delegate: Rectangle {
                    property SurfaceItem source: modelData.item
                    property bool highlighted: globalIndex == root.current
                    property real ratio: source.width / source.height
                    onRatioChanged: {
                        eqhgrid.calcLayout()
                    }

                    width: displayWidth
                    height: col.height + 2 * padding
                    border.color: "blue"
                    border.width: highlighted ? 2 : 0
                    color: Qt.rgba(255, 255, 255, .2)
                    radius: 8
                    
                    onHighlightedChanged: {
                        // auto scroll to current highlight
                        if (highlighted) {
                            flickable.contentY = Math.min(
                                        Math.max(
                                            mapToItem(eqhgrid, 0, height).y - flickable.height,
                                            flickable.contentY
                                        ), mapToItem(eqhgrid, 0, 0).y)
                        }
                    }
                    Column {
                        id: col
                        anchors {
                            left: parent.left
                            right: parent.right
                            top: parent.top
                            margins: root.padding
                        }
                        spacing: 5

                        RowLayout {
                            id: titlebar
                            width: parent.width
                            height: 24
                            Rectangle {
                                height: parent.height
                                width: height
                                color: "yellow"
                                radius: 5
                            }
                            Text {
                                Layout.fillWidth: true
                                text: {
                                    const xdg = source.shellSurface
                                    const wholeTitle = xdg.appId?.length ? `${xdg.title} - ${xdg.appId}` : xdg.title
                                    wholeTitle
                                }
                                elide: Qt.ElideRight
                            }
                        }
                        Item {
                            id: thumb
                            width: parent.width
                            height: source.height * width / source.width
                            clip: true
                            visible: true
                            ShaderEffectSource {
                                anchors.centerIn: parent
                                width: parent.width
                                height: source.height * width / source.width
                                live: true
                                hideSource: false
                                smooth: true
                                sourceItem: source
                            }
                        }
                    }
                    TapHandler {
                        onTapped: {
                            root.current = globalIndex
                        }
                    }
                }
            }
        }
        function calcLayout(){
            eqhgrid.calcLayout()
        }
    }
}

// Copyright (C) 2024 Yicheng Zhong <zhongyicheng@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import Waylib.Server
import TreeLand
import TreeLand.Protocols
import TreeLand.Utils

FocusScope {
    id: root
    required property int workspaceId
    required property int workspaceRelativeId
    property alias surfaces: surfacesModel

    Item {
        id: dbg
        anchors {
            top: parent.top
            right: parent.right
        }
        width: 200
        Column {
            Text {
                text: `No.${workspaceRelativeId} wsid: ${workspaceId}`
                color: "white"
            }
        }
    }

    ListModel {
        id: surfacesModel
        function removeIf(cond) {
            for (let i = 0; i < this.count; i++) {
                if (cond(this.get(i))) {
                    this.remove(i)
                    return
                }
            }
        }
        function findIf(cond) {
            for (let i = 0; i < this.count; i++) {
                if (cond(this.get(i))) {
                    return i
                }
            }
            return -1
        }
        function appendEnhanced(data) {
            // ensure activated surface is on top, activatedSurfaceChanged may happen before model append
            if (data.item.shellSurface === Helper.activatedSurface) {
                this.insert(0,data)
            }
            else
                this.append(data)
        }
    }

    Connections {
        target: Helper
        function onActivatedSurfaceChanged() {
            if (Helper.activatedSurface !== null) {
                // adjust window stack
                const activatedSurfaceIndex = surfacesModel.findIf((data) => data.item.shellSurface === Helper.activatedSurface)
                if (activatedSurfaceIndex >= 0)
                    surfacesModel.move(activatedSurfaceIndex, 0, 1)
            }
        }
    }

    Component.onDestruction: {
        const moveToRelId = 0
        const moveTo = QmlHelper.workspaceManager.layoutOrder.get(moveToRelId).wsid
        let temp = Array.from(children) // immutable when iterating
        for (let child of temp) {
            if(child instanceof SurfaceWrapper) {
                child.workspaceId = moveTo
            }
        }
    }

    function selectSurfaceToActivate() {
        const nextIndex = surfacesModel.findIf( (data) => Helper.selectSurfaceToActivate(data.item.shellSurface))
        if (nextIndex >= 0)
            Helper.activatedSurface = surfacesModel.get(nextIndex).item.shellSurface
        else Helper.activatedSurface = null
    }
}

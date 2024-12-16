// Copyright (C) 2024 Yicheng Zhong <zhongyicheng@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property int minH
    required property int maxH
    required property int maxW
    required property int availH
    required property int availW
    required property var model
    required property Component delegate
    
    property int itemVerticalPadding: 0
    property int spacing: 8
    property bool enterAnimationEnabled: false
    property bool enterAnimationFinished: false

    property var pos: []

    Repeater {
        model: root.model
        Loader {
            required property int index
            required property var modelData
            property var internalData: pos[index]
            property int globalIndex: index
            property real displayWidth: modelData.width
            property bool initialized: false
            property bool actv: root.enterAnimationEnabled && !root.enterAnimationFinished && initialized && pos.length > index
            sourceComponent: root.delegate
            function initialize() {
                return mapFromItem(modelData, 0, 0)
            }
            x: { return initialize().x }
            y: { return initialize().y }
            z: -index
            onInternalDataChanged: if (internalData && internalData.dw && initialized) {
                x = internalData.dx
                y = internalData.dy
                displayWidth = internalData.dw
            }
            Behavior on x { enabled: actv; NumberAnimation { duration: 250; easing.type: Easing.OutCubic }}
            Behavior on y { enabled: actv; NumberAnimation { duration: 250; easing.type: Easing.OutCubic }}
            Behavior on displayWidth {
                enabled: actv
                SequentialAnimation {
                    NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                    ScriptAction {
                        script: root.enterAnimationFinished = true
                    }
                }
            }
            Component.onCompleted: {
                initialized = true
            }
        }
        // caution: repeater's remove may happen after calclayout, so last elem got null and some got wrong sourceitem
        onItemAoceand: {
            calcLayout()
        }
        onItemRemoved: calcLayout()
    }
    property var getRatio: (d)=>d.source.width / d.source.height
    function calcLayout() {
        let rows = []
        let rowHeight = 0
        function tryLayout(rowH, div) {
            var nrows = 1
            var acc = 0
            var rowstmp = []
            var currow = []
            for (var i = 0; i < model.count; i++) {
                var win = model.get(i)
                var ratio = getRatio(win)
                var curW = Math.min(maxW, ratio * rowH)
                const displayInfo = {dw: curW}
                acc += curW + root.spacing
                if (acc <= availW)
                    currow.push(displayInfo)
                else {
                    acc = curW
                    nrows++
                    rowstmp.push(currow)
                    currow = [displayInfo]
                    if (nrows > div)
                        break
                }
            }
            if (nrows <= div) {
                if (currow.length)
                    rowstmp.push(currow)
                rowHeight = rowH
                rows = rowstmp
                return true
            }
            return false
        }
        function calcDisplayPos() {
            let postmp = []
            let curY = 0
            const maxW = rows.reduce((acc, row) => Math.max(acc, row.reduce( (acc, it) => it.dw + acc + root.spacing, -root.spacing )), 0)
            root.width = maxW
            const hCenter = root.width / 2
            for (let row of rows) {
                const totW = row.reduce((acc, it) => it.dw + acc + root.spacing, -root.spacing)
                const left = hCenter - totW / 2
                row.reduce((acc, it) => {
                    Object.assign(it, {dx: acc, dy: curY})
                    postmp.push(it)
                    return it.dw + acc + root.spacing
                }, left)
                curY += rowHeight + itemVerticalPadding + root.spacing
            }
            pos = postmp
            root.height = curY - root.spacing
            return
        }
        for (var div = 1; availH / div >= minH; div++) {
            // return if width satisfies
            var rowH = Math.min(availH / div, maxH)
            if (tryLayout(rowH, div)) {
                calcDisplayPos()
                return
            }
        }
        if (tryLayout(minH, 999)) {
            calcDisplayPos()
        } else {
            console.warn('cannot layout')
        }
    }
}

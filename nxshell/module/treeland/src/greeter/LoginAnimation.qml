// Copyright (C) 2023 justforlxz <justforlxz@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick

Item {
    id: root

    signal stopped

    visible: false

    function start(target, pos, to) {
        width = target.width
        height = target.height

        effect.sourceItem = target
        effect.width = target.width
        effect.height = target.height

        xAni.from = pos.x
        xAni.to = to.x

        visible = true
        animation.start()
    }

    function stop() {
        visible = false
        effect.sourceItem = null
        stopped()
    }

    Item {
        id: content

        width: parent.width
        height: parent.height
        clip: true

        ShaderEffectSource {
            id: effect
            live: false
            hideSource: true
        }
    }

    Connections {
        target: animation
        function onStopped() {
            stop()
        }
    }

    ParallelAnimation {
        id: animation

        PropertyAnimation {
            id: xAni
            target: content
            property: "x"
            duration: 800
            easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: content
            property: "opacity"
            duration: 600
            from: 1
            to: 0
            easing.type: Easing.InQuad
        }
    }
}


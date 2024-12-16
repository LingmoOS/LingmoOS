// Copyright (C) 2023 justforlxz <justforlxz@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick

Item {
    id: root

    signal stopped

    visible: false

    required property var target

    width: target.width
    height: target.height

    function start() {
        visible = true;
        animation.start();
    }

    function stop() {
        visible = false;
        effect.sourceItem = null;
        stopped();
    }

    Item {
        id: content

        width: parent.width
        height: parent.height
        clip: true

        transform: [
            Rotation {
                id: rotation
                origin.x: width / 2
                origin.y: height / 2
                axis {
                    x: 1
                    y: 0
                    z: 0
                }
                angle: 0
            },
            Scale {
                id: scale
                origin.x: width / 2
                origin.y: height / 2
                xScale: 1
                yScale: 1
            }
        ]

        ShaderEffectSource {
            id: effect
            live: true
            hideSource: true
            sourceItem: target
            width: target.width
            height: target.height
        }
    }

    Connections {
        target: animation
        function onStopped() {
            stop();
        }
    }

    ParallelAnimation {
        id: animation

        PropertyAnimation {
            target: rotation
            property: "angle"
            duration: 400
            from: 72
            to: 0
            easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: scale
            property: "xScale"
            duration: 400
            from: 0.4
            to: 1
            easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: scale
            property: "yScale"
            duration: 400
            from: 0.4
            to: 1
            easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: content
            property: "opacity"
            duration: 400
            from: 0.2
            to: 1
            easing.type: Easing.InQuad
        }
    }
}

// Copyright (C) 2023 justforlxz <justforlxz@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Particles

Item {
    id: root

    signal stopped()

    visible: false

    function start(target) {
        width = target.width
        height = target.height
        effect.sourceItem = target
        effect.width = target.width
        effect.height = target.height
        visible = true
        hideAnimation.start()
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

    PropertyAnimation {
        id: hideAnimation
        duration: 500
        target: content
        from: parent.height
        to: 0
        property: "height"

        onStopped: {
            root.stopped()
        }
    }

    // Copy from the Qt particles example "affectors"
    ParticleSystem {
        id: flameSystem

        anchors.fill: parent

        ImageParticle {
            groups: ["flame"]
            source: "qrc:///particleresources/glowdot.png"
            color: "#88ff200f"
            colorVariation: 0.1
        }

        Row {
            width: parent.width
            anchors {
                bottom: parent.bottom
                bottomMargin: parent.height - content.height
            }

            Repeater {
                model: parent.width / 50

                Emitter {
                    system: flameSystem
                    group: "flame"
                    width: 50
                    height: 50

                    emitRate: 120
                    lifeSpan: 200
                    size: 50
                    endSize: 10
                    sizeVariation: 10
                    acceleration: PointDirection { y: -40 }
                    velocity: AngleDirection { angle: 270; magnitude: 20; angleVariation: 22; magnitudeVariation: 5 }
                }
            }
        }
    }
}

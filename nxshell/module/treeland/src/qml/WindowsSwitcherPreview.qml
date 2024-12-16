// Copyright (C) 2023 Dingyuan Zhang <zhangdingyuan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick

Item {
    id: root

    visible: false

    function start(target) {
        width = target.width
        height = target.height
        x = target.x
        y = target.y

        effect.sourceItem = target
        effect.width = target.width
        effect.height = target.height

        visible = true
    }

    function stop() {
        visible = false
        effect.sourceItem = null
    }

    Item {
        id: content

        width: parent.width
        height: parent.height
        clip: true

        ShaderEffectSource {
            id: effect
            live: true
            hideSource: true
        }
    }
}



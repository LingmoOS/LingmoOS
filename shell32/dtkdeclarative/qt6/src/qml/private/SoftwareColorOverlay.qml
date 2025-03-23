// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import Qt5Compat.GraphicalEffects.private
import org.deepin.dtk 1.0 as D

Item {
    id: rootItem

    property variant source
    property alias color: overlay.color
    property alias cached: overlay.cached

    SourceProxy {
        id: sourceProxy
        input: rootItem.source
    }

    ShaderEffectSource {
        id: cacheItem
        anchors.fill: parent
        visible: rootItem.cached
        smooth: true
        sourceItem: overlay
        live: true
        hideSource: visible
    }

    D.SoftwareColorOverlay {
        id: overlay
        anchors.fill: parent
        source: sourceProxy.output
    }
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtGraphicalEffects.private 1.0
import org.deepin.dtk.impl 1.0 as D

/*!
    The Qt Graphical Effects module provides a SourceProxy types.
    the user can simply use ShaderEffectSource or Item as input

*/
Item {
    id: rootItem

    property variant source
    property variant maskSource
    property bool cached: false
    property alias invert: mask.invert

    SourceProxy {
        id: sourceProxy
        input: rootItem.source
    }

    SourceProxy {
        id: maskSourceProxy
        input: rootItem.maskSource
    }

    ShaderEffectSource {
        id: cacheItem
        anchors.fill: parent
        visible: rootItem.cached
        smooth: true
        sourceItem: mask
        live: true
        hideSource: visible
    }

    D.SoftwareOpacityMask {
        id: mask
        anchors.fill: parent
        source: sourceProxy.output
        maskSource: maskSourceProxy.output
    }
}

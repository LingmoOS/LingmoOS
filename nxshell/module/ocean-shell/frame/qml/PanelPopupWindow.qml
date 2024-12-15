// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.lingmo.ds 1.0
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DStyle

PopupWindow {
    id: root

    property real xOffset: 0
    property real yOffset: 0
    property int margins: 10
    property Item currentItem
    signal requestUpdateGeometry()
    signal updateGeometryFinished()

    // order to update screen and (x,y)
    property var updateGeometryer : function updateGeometry()
    {
        if (root.width <= 10 || root.height <= 10) {
            return
        }
        if (!root.transientParent)
            return

        // following transientParent's screen.
        root.screen = root.transientParent.screen

        let bounding = Qt.rect(root.screen.virtualX + margins, root.screen.virtualY + margins,
                               root.screen.width - margins * 2, root.screen.height - margins * 2)
        let pos = Qt.point(transientParent ? transientParent.x + xOffset : xOffset,
                           transientParent ? transientParent.y + yOffset : YOffset)
        x = selectValue(pos.x, bounding.left, bounding.right - root.width)
        y = selectValue(pos.y, bounding.top, bounding.bottom - root.height)
    }

    function selectValue(value, min, max) {
        // wayland do not need to be limitted in the screen, this has been done by compositor
        if (Qt.platform.pluginName === "wayland")
            return value

        if (value < min)
            return min
        if (value > max)
            return max

        return value
    }

    // FIXME: The contentItem of QQuickWindow originally maintains the same size as the Window in the resizeEvent of QQuickWindow,
    // but there will be inconsistencies under Wayland. Maybe it is a bug of QtWayland.
    Binding {
        target: root.contentItem
        property: "width"
        value: root.width
    }

    Binding {
        target: root.contentItem
        property: "height"
        value: root.height
    }

    width: 10
    height: 10
    flags: (Qt.platform.pluginName === "xcb" ?  (Qt.Tool | Qt.WindowStaysOnTopHint) : Qt.Popup)
    D.DWindow.enabled: true
    D.DWindow.windowRadius: D.DTK.platformTheme.windowRadius < 0 ? 4 : D.DTK.platformTheme.windowRadius
    D.DWindow.enableSystemResize: false
    D.DWindow.enableSystemMove: false
    D.DWindow.enableBlurWindow: true
    // TODO set shadowOffset maunally.
    D.DWindow.shadowOffset: Qt.point(0, 25)
    D.DWindow.shadowColor: D.DTK.themeType === D.ApplicationHelper.DarkType ? Qt.rgba(0, 0, 0, 0.5) : Qt.rgba(0, 0, 0, 0.2)
    D.ColorSelector.family: D.Palette.CrystalColor

    color: "transparent"
    onVisibleChanged: function (arg) {
        if (!arg)
            currentItem = null
        if (!arg)
            DS.closeChildrenWindows(root)
    }

    Connections {
        target: root.transientParent
        function onScreenChanged()
        {
            requestUpdateGeometry()
        }
        function onYChanged()
        {
            requestUpdateGeometry()
        }
        function onXChanged()
        {
            requestUpdateGeometry()
        }
    }

    Connections {
        target: root.Screen
        function onDesktopAvailableHeightChanged()
        {
            requestUpdateGeometry()
        }
        function onDesktopAvailableWidthChanged()
        {
            requestUpdateGeometry()
        }
    }

    onHeightChanged: requestUpdateGeometry()
    onWidthChanged: requestUpdateGeometry()
    onXOffsetChanged: requestUpdateGeometry()
    onYOffsetChanged: requestUpdateGeometry()

    onRequestUpdateGeometry: {
        if (updateGeometryer) {
            Qt.callLater(function () {
                updateGeometryer()
                updateGeometryFinished()
            })
        }
    }

    D.StyledBehindWindowBlur {
        control: parent
        anchors.fill: parent
        function blendColorAlpha(fallback) {
            var appearance = DS.applet("org.lingmo.ds.ocean-appearance")
            if (!appearance || appearance.opacity < 0)
                return fallback
            return appearance.opacity
        }
        blendColor: {
            if (valid) {
                return DStyle.Style.control.selectColor(undefined,
                                                    Qt.rgba(235 / 255.0, 235 / 255.0, 235 / 255.0, blendColorAlpha(0.6)),
                                                    Qt.rgba(0, 0, 0, blendColorAlpha(85 / 255)))
            }
            return DStyle.Style.control.selectColor(undefined,
                                                DStyle.Style.behindWindowBlur.lightNoBlurColor,
                                                DStyle.Style.behindWindowBlur.darkNoBlurColor)
        }
    }
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Shapes 1.11
import QtQuick.Templates 2.4 as T
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.Slider {
    id: control

    enum HandleType {
        NoArrowHorizontal = 0,
        NoArrowVertical = 1,
        ArrowUp = 2,
        ArrowLeft = 3,
        ArrowBottom = 4,
        ArrowRight = 5
    }

    property D.Palette grooveColor: DS.Style.slider.groove.background

    property alias handleType: __handle.type
    property real dashOffset: 0.0
    property var dashPattern: [0.5, 0.25]
    property bool highlightedPassedGroove: false

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            (handle ? handle.implicitWidth : 0) + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             (handle ? handle.implicitHeight : 0) + topPadding + bottomPadding)
    opacity: control.D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1

    // draw handle
    handle: SliderHandle {
        id: __handle
        x: control.leftPadding + (control.horizontal ? control.visualPosition * (control.availableWidth - width) : 0)
        y: control.topPadding + (control.horizontal ? 0 : control.visualPosition * (control.availableHeight - height))
        width: control.horizontal ? DS.Style.slider.handle.width : DS.Style.slider.handle.height
        height: control.horizontal ? DS.Style.slider.handle.height : DS.Style.slider.handle.width
        palette: D.DTK.makeIconPalette(control.palette)
        mode: control.D.ColorSelector.controlState
        theme: control.D.ColorSelector.controlTheme
    }

    // draw panel
    background: Item {
        anchors {
            horizontalCenter: !control.horizontal ? control.handle.horizontalCenter : undefined
            verticalCenter: control.horizontal ? control.handle.verticalCenter : undefined
        }

        implicitWidth: control.horizontal ? DS.Style.slider.width : handle.width
        implicitHeight: control.horizontal ? handle.height : DS.Style.slider.height

        // draw groove
        Item {
            id: sliderGroove
            x: control.horizontal ? 0 : (parent.width - width) / 2
            y: control.horizontal ? (parent.height - height) / 2 : 0
            width: control.horizontal ? parent.width : DS.Style.slider.groove.height
            height: control.horizontal ? DS.Style.slider.groove.height : parent.height
            Shape {
                ShapePath {
                    capStyle: ShapePath.FlatCap
                    strokeStyle: ShapePath.DashLine
                    strokeColor: control.D.ColorSelector.grooveColor
                    strokeWidth: control.horizontal ? sliderGroove.height : sliderGroove.width
                    dashOffset: control.dashOffset
                    dashPattern: control.dashPattern
                    startX: control.horizontal ? 0 : sliderGroove.width / 2
                    startY: control.horizontal ? sliderGroove.height / 2 : 0
                    PathLine {
                        x: control.horizontal ? sliderGroove.width : sliderGroove.width / 2
                        y: control.horizontal ? sliderGroove.height / 2 : sliderGroove.height
                    }
                }
            }

            // draw passed groove area
            Loader {
                active: highlightedPassedGroove
                sourceComponent: Shape {
                    ShapePath {
                        capStyle: ShapePath.FlatCap
                        strokeStyle: ShapePath.DashLine
                        strokeColor: control.palette.highlight
                        strokeWidth: control.horizontal ? sliderGroove.height : sliderGroove.width
                        dashOffset: control.dashOffset
                        dashPattern: control.dashPattern
                        startX: control.horizontal ? 0 : sliderGroove.width / 2
                        startY: control.horizontal ? sliderGroove.height / 2 : sliderGroove.height
                        PathLine {
                            x: control.horizontal ? control.handle.x : sliderGroove.width / 2
                            y: control.horizontal ? sliderGroove.height / 2 : control.handle.y + control.handle.height / 2
                        }
                    }

                    Item {
                        id: passItem
                        y: control.horizontal ? -DS.Style.slider.groove.height / 2 : control.handle.y + control.handle.height / 2
                        height: control.horizontal ? DS.Style.slider.groove.height : sliderGroove.height - control.handle.y - control.handle.height / 2
                        width: control.horizontal ? control.handle.x : DS.Style.slider.groove.height
                    }

                    BoxShadow {
                        anchors.fill: passItem
                        shadowBlur: 4
                        shadowOffsetY: 2
                        shadowColor: control.palette.highlight
                        rotation: control.horizontal ? 0 : 180
                        opacity: 0.25
                    }
                }
            }
        }
    }
}

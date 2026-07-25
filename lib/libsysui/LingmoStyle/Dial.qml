/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */
import QtQuick
import QtQuick.Templates as T
import LingmoUI

T.Dial {
    property color hoverColor: LingmoTheme.dark ? Qt.rgba(
                                                      68 / 255, 68 / 255,
                                                      68 / 255,
                                                      1) : Qt.rgba(246 / 255, 246
                                                                   / 255, 246 / 255, 1)

    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    background: T.ProgressBar {
        property int duration: 2000
        property real strokeWidth: 6
        property bool progressVisible: true
        property color color: LingmoTheme.primaryColor
        property color backgroundColor: LingmoTheme.dark ? Qt.rgba(
                                                               99 / 255,
                                                               99 / 255,
                                                               99 / 255,
                                                               1) : Qt.rgba(214 / 255, 214 / 255, 214 / 255, 1)
        id: pg_back

        implicitWidth: 100
        implicitHeight: 100

        x: control.width / 2 - width / 2
        y: control.height / 2 - height / 2
        width: Math.max(64, Math.min(control.width, control.height))
        height: width

        indeterminate: false
        clip: true
        background: Rectangle {
            implicitWidth: 56
            implicitHeight: 56
            radius: pg_back.width / 2
            color: "transparent"
            border.color: pg_back.backgroundColor
            border.width: pg_back.strokeWidth
        }
        onIndeterminateChanged: {
            canvas.requestPaint()
        }

        value: {
            return control.value / (control.to - control.from)
        }

        QtObject {
            id: d
            property real _radius: pg_back.width / 2 - pg_back.strokeWidth / 2
            property real _progress: pg_back.indeterminate ? 0.0 : pg_back.visualPosition
            on_ProgressChanged: {
                canvas.requestPaint()
            }
        }
        Connections {
            target: LingmoTheme
            function onDarkChanged() {
                canvas.requestPaint()
            }
        }
        contentItem: Item {
            id: layout_item
            Canvas {
                id: canvas
                anchors.fill: parent
                antialiasing: true
                renderTarget: Canvas.Image
                property real startAngle: 0
                property real sweepAngle: 0
                SequentialAnimation on startAngle {
                    loops: Animation.Infinite
                    running: pg_back.visible && pg_back.indeterminate
                    PropertyAnimation {
                        from: 0
                        to: 450
                        duration: pg_back.duration / 2
                    }
                    PropertyAnimation {
                        from: 450
                        to: 1080
                        duration: pg_back.duration / 2
                    }
                }
                SequentialAnimation on sweepAngle {
                    loops: Animation.Infinite
                    running: pg_back.visible && pg_back.indeterminate
                    PropertyAnimation {
                        from: 0
                        to: 180
                        duration: pg_back.duration / 2
                    }
                    PropertyAnimation {
                        from: 180
                        to: 0
                        duration: pg_back.duration / 2
                    }
                }
                onStartAngleChanged: {
                    requestPaint()
                }
                onPaint: {
                    var ctx = canvas.getContext("2d")
                    ctx.clearRect(0, 0, canvas.width, canvas.height)
                    ctx.save()
                    ctx.lineWidth = pg_back.strokeWidth
                    ctx.strokeStyle = pg_back.color
                    ctx.lineCap = "round"
                    ctx.beginPath()
                    if (pg_back.indeterminate) {
                        ctx.arc(width / 2, height / 2,
                                d._radius, Math.PI * (startAngle - 90) / 180, Math.PI
                                * (startAngle - 90 + sweepAngle) / 180)
                    } else {
                        ctx.arc(width / 2, height / 2, d._radius,
                                control.startAngle - 90,
                                control.startAngle - 90 + d._progress * 2 * Math.PI)
                    }
                    ctx.stroke()
                    ctx.closePath()
                    ctx.restore()
                }
            }
        }
        LingmoText {
            text: (pg_back.visualPosition * 100).toFixed(0) + "%"
            visible: {
                if (pg_back.indeterminate) {
                    return false
                }
                return pg_back.progressVisible
            }
            anchors.centerIn: parent
        }
    }

    handle: Rectangle {

        x: control.background.x + control.background.width / 2 - width / 2
        y: control.background.y + control.background.height / 2 - height / 2

        implicitWidth: 20
        implicitHeight: 20
        radius: 10
        color: LingmoTheme.dark ? Qt.rgba(69 / 255, 69 / 255, 69 / 255,
                                          1) : Qt.rgba(1, 1, 1, 1)
        LingmoShadow {
            radius: 10
        }
        LingmoIcon {
            width: 10
            height: 10
            Behavior on scale {
                NumberAnimation {
                    duration: 167
                    easing.type: Easing.OutCubic
                }
            }
            iconSource: LingmoIcons.FullCircleMask
            iconSize: 10
            scale: {
                if (control.pressed) {
                    return 0.9
                }
                return control.hovered ? 1.2 : 1
            }
            iconColor: LingmoTheme.primaryColor
            anchors.centerIn: parent
        }

        transform: [
            Translate {
                y: -control.background.height * 0.4
                   + (control.handle ? control.handle.height / 2 : 0)
            },
            Rotation {
                angle: control.angle
                origin.x: control.handle ? control.handle.width / 2 : 0
                origin.y: control.handle ? control.handle.height / 2 : 0
            }
        ]
    }
}

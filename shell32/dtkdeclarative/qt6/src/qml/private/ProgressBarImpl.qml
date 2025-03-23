// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls.impl 2.4
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk 1.0

Item {
    id: control
    property Item progressBar
    property string formatText
    property bool animationStop: false
    property D.Palette shadowPaletteColor: DS.Style.progressBar.shadowColor
    property D.Palette handleGradientColor: DS.Style.progressBar.handleGradientColor

    Loader {
        anchors.fill: parent
        sourceComponent: progressBar.indeterminate ? _indeterminateComponent
                                                   : _valueComponent
    }

    Component {
        id: _valueComponent

        Item {
            BoxShadow {
                y: (parent.height - height) / 2
                x: -y
                width: progressBar.height
                height: progressBar.visualPosition * progressBar.width
                shadowOffsetX: -4
                shadowBlur: 6
                rotation: -90
                cornerRadius: DS.Style.control.radius
                shadowColor: control.D.ColorSelector.shadowPaletteColor
                visible: progressBar.visualPosition > 0

                Rectangle {
                    id: rect
                    anchors.fill: parent
                    radius: parent.cornerRadius
                    property int count
                    property real lightPosition
                    gradient: Gradient {
                        GradientStop { position: 0; color: progressBar.palette.highlight }
                        GradientStop { position: rect.lightPosition; color: control.D.ColorSelector.handleGradientColor }
                        GradientStop { position: 1; color: progressBar.palette.highlight }
                    }
                    Timer {
                        id: moveTimer
                        interval: 10
                        repeat: true
                        running: rect.visible
                        onTriggered: {
                            moveTimer.interval = 10
                            if (rect.count === 100) {
                                rect.count = 0
                                rect.lightPosition = 0.0
                                moveTimer.interval = 2000
                                return;
                            }
                            rect.count += 1
                            rect.lightPosition = rect.count * 0.01
                        }
                    }
                }
            }

            ItemGroup {
                anchors.fill: parent
                ClippedText {
                    clip: progressBar.visualPosition > 0
                    clipX: -progressBar.leftPadding + progressBar.visualPosition * control.width
                    clipWidth: (1.0 - progressBar.visualPosition) * control.width
                    text: formatText
                    font: progressBar.font
                    color: progressBar.palette.buttonText
                    opacity: enabled ? 1 : 0.4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                ClippedText {
                    visible: progressBar.visualPosition > 0
                    clip: progressBar.visualPosition > 0
                    clipX: -progressBar.leftPadding
                    clipWidth: progressBar.visualPosition * control.width
                    text: formatText
                    font: progressBar.font
                    color: palette.highlightedText
                    opacity: enabled ? 1 : 0.4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
        }
    }

    Component {
        id: _indeterminateComponent
        Item {
            BoxShadow {
                id: indeterminateProgressContent
                property real limitWidth: (indeterminateProgressContent.height - indeterminateProgressContent.width) / 2
                y: -limitWidth
                x: limitWidth
                implicitHeight: DS.Style.progressBar.indeterminateProgressBarWidth
                width: progressBar.height
                shadowOffsetX: -4
                shadowBlur: 6
                cornerRadius: DS.Style.control.radius
                shadowColor: control.D.ColorSelector.shadowPaletteColor
                rotation: -90

                Rectangle {
                    id: indeterminateRect
                    anchors.fill: parent
                    radius: indeterminateProgressContent.cornerRadius
                    property int count
                    property real lightPosition
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: progressBar.palette.highlight }
                        GradientStop { position: indeterminateRect.lightPosition; color: control.D.ColorSelector.handleGradientColor }
                        GradientStop { position: 1.0; color: progressBar.palette.highlight }
                    }
                    Timer {
                        id: indeterminateMoveTimer
                        interval: 50
                        repeat: true
                        running: indeterminateRect.visible
                        onTriggered: {
                            indeterminateMoveTimer.interval = 50
                            if (indeterminateRect.count === 100) {
                                indeterminateRect.count = 0
                                indeterminateMoveTimer.interval = 2000
                                return;
                            }
                            indeterminateRect.count += 5
                            indeterminateRect.lightPosition = indeterminateRect.count * 0.01
                        }
                    }
                }

                SequentialAnimation {
                    id: indeterminateAnimation
                    running: progressBar.indeterminate && !control.animationStop
                    loops: Animation.Infinite
                    NumberAnimation {
                        target: indeterminateProgressContent
                        property: "x"
                        from: -indeterminateProgressContent.y
                        to: progressBar.width - indeterminateProgressContent.limitWidth - indeterminateProgressContent.width
                        duration: DS.Style.progressBar.indeterminateProgressBarAnimationDuration * progressBar.width / DS.Style.progressBar.width
                        onFromChanged: if (!delayAnimationTimer.running) delayAnimationTimer.start()
                        onToChanged:  if (!delayAnimationTimer.running) delayAnimationTimer.start()
                    }

                    NumberAnimation {
                        target: indeterminateProgressContent
                        property: "x"
                        to: -indeterminateProgressContent.y
                        duration: DS.Style.progressBar.indeterminateProgressBarAnimationDuration * progressBar.width / DS.Style.progressBar.width
                        onFromChanged: if (!delayAnimationTimer.running) delayAnimationTimer.start()
                        onToChanged: if (!delayAnimationTimer.running) delayAnimationTimer.start()
                    }
                }
            }

            Timer {
                // ###(Chen Bin) Layout bug: Put the progressbar into the layout,
                // the animation will get a wrong position
                id: delayAnimationTimer
                interval: 200
                repeat: false
                onTriggered: {
                    indeterminateAnimation.restart()
                }
            }

            ItemGroup {
                anchors.fill: parent
                ClippedText {
                    clip: true
                    clipX: indeterminateProgressContent.x - indeterminateProgressContent.limitWidth - progressBar.leftPadding
                    clipWidth: indeterminateProgressContent.height
                    text: formatText
                    font: progressBar.font
                    color: palette.highlightedText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                ClippedText {
                    clip: true
                    visible: indeterminateProgressContent.x - indeterminateProgressContent.limitWidth + progressBar.leftPadding > 0
                    clipX: -progressBar.leftPadding
                    clipWidth: indeterminateProgressContent.x - indeterminateProgressContent.limitWidth + progressBar.leftPadding
                    text: formatText
                    font: progressBar.font
                    color: progressBar.palette.buttonText
                    opacity: enabled ? 1 : 0.4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                ClippedText {
                    clip: true
                    clipX: indeterminateProgressContent.x - indeterminateProgressContent.limitWidth - progressBar.leftPadding + indeterminateProgressContent.height
                    clipWidth: progressBar.width - progressBar.rightPadding - indeterminateProgressContent.x - indeterminateProgressContent.limitWidth
                               - progressBar.leftPadding + indeterminateProgressContent.height
                    text: formatText
                    font: progressBar.font
                    color: progressBar.palette.buttonText
                    opacity: enabled ? 1 : 0.4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
        }
    }
}

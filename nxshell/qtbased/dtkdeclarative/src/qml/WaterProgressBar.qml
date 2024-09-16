// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Control {
    id: control
    property int value /*0~100*/
    property bool running: false
    property D.Palette backgroundColor1: DS.Style.waterProgressBar.background1
    property D.Palette backgroundColor2: DS.Style.waterProgressBar.background2
    property D.Palette dropShadowColor: DS.Style.waterProgressBar.dropShadow
    property D.Palette popBackgroundColor: DS.Style.waterProgressBar.popBackground
    property D.Palette textColor: DS.Style.waterProgressBar.textColor

    background: BoxShadow {
        implicitWidth: DS.Style.waterProgressBar.width
        implicitHeight: DS.Style.waterProgressBar.height
        shadowColor: control.D.ColorSelector.dropShadowColor
        shadowOffsetY: 6
        shadowBlur: 6
        cornerRadius: width / 2

        Rectangle {
            anchors.fill: parent
            radius: parent.cornerRadius
            gradient: Gradient {
                GradientStop { position: 0.0; color: control.D.ColorSelector.backgroundColor1 }
                GradientStop { position: 1; color: control.D.ColorSelector.backgroundColor2 }
            }
        }
    }

    contentItem: Item {
        Item {
            id: content
            anchors.fill: parent
            D.WaterProgressAttribute {
                id: attribute
                running: control.running && control.value > 0
                waterProgress: control
                imageWidth: 5 * control.width
                imageHeight: 1.1 * control.height
            }

            Repeater {
                model: 4
                Image {
                    readonly property real xoffset: index < 2 ? attribute.backXOffset : attribute.frontXOffset
                    source: index < 2 ? DS.Style.waterProgressBar.waterBackImagePath
                                      : DS.Style.waterProgressBar.waterFrontImagePath
                    x: index % 2 ? xoffset - width : xoffset
                    y: (90 - control.value) * control.height / 100
                    width: attribute.imageWidth
                    height: attribute.imageHeight
                    sourceSize {
                        width: attribute.imageWidth
                        height: attribute.imageHeight
                    }
                }
            }

            Repeater {
                model: attribute.pops

                Loader {
                    x: model.x
                    y: model.y
                    width: model.width
                    height: model.height
                    active: control.value > 30
                    visible: active

                    sourceComponent: Rectangle {
                        color: control.D.ColorSelector.popBackgroundColor
                        radius: width / 2
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: control.value
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                font {
                    pixelSize: control.height * 0.4
                }
                color: control.D.ColorSelector.textColor
            }
        }

        D.ItemViewport {
            anchors.fill: parent
            sourceItem: content
            hideSource: true
            radius: width / 2
        }
    }
}

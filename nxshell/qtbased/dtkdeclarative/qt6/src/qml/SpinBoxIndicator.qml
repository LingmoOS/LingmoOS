// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS
import org.deepin.dtk.private as P

Control {
    id: control
    enum IndicatorDirection {
        UpIndicator = 0,
        DownIndicator = 1
    }

    property Item spinBox
    property bool pressed
    property bool singleIndicator: false
    property int direction
    property D.Palette inactiveBackgroundColor: DS.Style.spinBox.indicator.background

    palette.windowText: control.D.ColorSelector.inactiveBackgroundColor
    hoverEnabled: true
    implicitWidth: DS.Style.spinBox.indicator.width
    implicitHeight: spinBox.activeFocus ? spinBox.implicitHeight / 2 : DS.Style.spinBox.indicator.height
    opacity: D.ColorSelector.controlState === D.DTK.DisabledState ? 0.4 : 1

    Component {
        id: inactiveComponent
        D.DciIcon {
            id: icon
            sourceSize.width: DS.Style.spinBox.indicator.iconSize
            palette: D.DTK.makeIconPalette(control.palette)
            name: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? "entry_spinbox_up" : "entry_spinbox_down"
            mode: control.D.ColorSelector.controlState
            theme: control.D.ColorSelector.controlTheme
            fallbackToQIcon: false
        }
    }

    Component {
        id: activeComponent
        Item {
            Item {
                id: btnBackground
                width: parent.width + DS.Style.control.radius
                height: parent.height + DS.Style.control.radius
                anchors {
                    right: parent.right
                    top: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? parent.top : undefined
                    bottom: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? undefined : parent.bottom
                }

                Button {
                    id: btn
                    anchors {
                        right: parent.right
                        top: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? parent.top : undefined
                        bottom: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? undefined : parent.bottom
                    }
                    width: DS.Style.spinBox.indicatorWidth
                    height: spinBox.implicitHeight / 2
                    icon {
                        name: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? "arrow_ordinary_up" : "arrow_ordinary_down"
                        width: DS.Style.spinBox.indicator.focusIconSize
                        height: DS.Style.spinBox.indicator.focusIconSize
                    }
                    activeFocusOnTab: false
                    opacity: 1
                    background: P.ButtonPanel {
                        button: btn
                        radius: 0
                    }
                    onClicked: {
                        if (direction === SpinBoxIndicator.IndicatorDirection.UpIndicator) {
                            spinBox.increase()
                        } else {
                            spinBox.decrease()
                        }
                    }
                }
            }

            D.ItemViewport {
                id: viewport
                sourceItem: btnBackground
                radius: DS.Style.control.radius
                fixed: true
                width: btnBackground.width
                height: btnBackground.height
                anchors {
                    right: parent.right
                    top: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? parent.top : undefined
                    bottom: direction === SpinBoxIndicator.IndicatorDirection.UpIndicator ? undefined : parent.bottom
                }
                hideSource: true
            }
        }

    }

    Loader {
        anchors.fill: parent
        sourceComponent: spinBox.activeFocus ? activeComponent : inactiveComponent
    }
}

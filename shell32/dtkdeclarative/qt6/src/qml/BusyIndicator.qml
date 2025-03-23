// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.BusyIndicator {
    id: control

    property D.Palette fillColor: DS.Style.busyIndicator.fillColor

    implicitWidth: DS.Style.busyIndicator.size
    implicitHeight: implicitWidth
    padding: width / DS.Style.busyIndicator.paddingFactor
    contentItem: Item {
        implicitWidth: DS.Style.busyIndicator.size
        implicitHeight: implicitWidth
        Image {
            id: indicatorSource
            anchors.fill: parent
            source: DS.Style.busyIndicator.spinnerSource
            sourceSize {
                width: parent.width
                height: parent.height
            }
            visible: false
        }

        D.ColorOverlay {
            anchors.fill: indicatorSource
            source: indicatorSource
            color: control.D.ColorSelector.fillColor

            RotationAnimator on rotation {
                from: 0
                to: 360
                duration: DS.Style.busyIndicator.animationDuration
                running: control.running
                loops: Animation.Infinite
            }
        }
    }
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import QtQml
import QtQuick.Shapes 1.10
import org.deepin.dtk 1.0

import org.deepin.album 1.0 as Album

Item {
    id: statusBar

    property int sliderValue: slider.value

    function setSliderWidgetValue(value) {
        slider.value = value
    }

    Label {
        height: parent.height
        anchors.centerIn: parent
        verticalAlignment: Text.AlignVCenter
        font: DTK.fontManager.t8
        text: GStatus.statusBarNumText
    }

    Slider {
        id: slider
        width: 160
        height: parent.height
        anchors {
            top: parent.top
            topMargin: 3
            right: parent.right
            rightMargin: 20
        }
        visible: !(GStatus.currentViewType === Album.Types.ViewCollecttion && GStatus.currentCollecttionViewIndex < 2)
        from: 0
        value: 0
        stepSize: 1
        to: 9
    }
}

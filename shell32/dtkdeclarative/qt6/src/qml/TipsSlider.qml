// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Control {
    enum TickDirection {
        Front = 0,
        Back = 1
    }

    property alias slider: __slider
    property alias ticks: ticksGrid.children
    property int tickDirection: (TipsSlider.TickDirection.Back)

    implicitWidth: __slider.width + (__slider.horizontal ? 0 : ticksGrid.childrenRect.width)
    implicitHeight: __slider.height + (__slider.horizontal ? ticksGrid.childrenRect.height : 0)

    Slider {
        id: __slider
        anchors {
            left: horizontal ? parent.left : (TipsSlider.TickDirection.Back === tickDirection ? parent.left : undefined)
            right: horizontal ? parent.right : (TipsSlider.TickDirection.Front === tickDirection ? parent.right : undefined)
            top: horizontal ? (TipsSlider.TickDirection.Back === tickDirection ? parent.top : undefined) : parent.top
            bottom: horizontal ? (TipsSlider.TickDirection.Front === tickDirection ? parent.bottom : undefined) : parent.bottom
        }
    }

    Grid {
        id: ticksGrid
        rows: __slider.horizontal ? 0 : children.length
        columns: __slider.horizontal ? children.length : 0

        anchors {
            left: __slider.horizontal ? parent.left : (TipsSlider.TickDirection.Back === tickDirection ? __slider.right : undefined)
            right: __slider.horizontal ? parent.right : (TipsSlider.TickDirection.Front === tickDirection ? __slider.left : undefined)
            leftMargin: __slider.horizontal ? __slider.handle.width / 2 : 0
            rightMargin: __slider.horizontal ? __slider.handle.width / 2 : 0
            top: __slider.horizontal ? (TipsSlider.TickDirection.Back === tickDirection ? __slider.bottom : undefined) : __slider.top
            bottom: __slider.horizontal ? (TipsSlider.TickDirection.Front === tickDirection ? __slider.top : undefined) : __slider.bottom
            topMargin: __slider.horizontal ? 0 : __slider.handle.height / 2
            bottomMargin: __slider.horizontal ? 0 : __slider.handle.height / 2
        }
        spacing: ticks.length > 1 ? (__slider.horizontal ? (parent.width - __slider.handle.width - DS.Style.slider.tick.width * ticks.length)
                                                         :(parent.height - __slider.handle.height - DS.Style.slider.tick.width * ticks.length)) / (ticks.length - 1) : 0
    }
}

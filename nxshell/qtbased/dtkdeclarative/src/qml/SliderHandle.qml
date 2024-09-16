// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk.impl 1.0 as D

D.DciIcon {
    id: control

    property int type: Slider.HandleType.NoArrowHorizontal

    function getIconNameByType(handleType) {
        switch(handleType) {
        case Slider.HandleType.NoArrowHorizontal:
            return "slider_round_hor"
        case Slider.HandleType.NoArrowVertical:
            return "slider_round_ver"
        case Slider.HandleType.ArrowUp:
            return "slider_point_up"
        case Slider.HandleType.ArrowBottom:
            return "slider_point_down"
        case Slider.HandleType.ArrowLeft:
            return "slider_point_left"
        case Slider.HandleType.ArrowRight:
            return "slider_point_left"
        }
    }

    sourceSize.width: control.width
    sourceSize.height: control.height
    name: getIconNameByType(type)
    mirror: Slider.HandleType.ArrowRight === control.type
    fallbackToQIcon: false
}

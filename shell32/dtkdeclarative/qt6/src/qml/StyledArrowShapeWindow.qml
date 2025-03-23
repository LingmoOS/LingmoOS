// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

ArrowShapeWindow {
    property alias control: blur.control
    D.DWindow.borderColor: DS.Style.control.selectColor((control ? control.palette.window : undefined),
                                                        DS.Style.arrowRectangleBlur.borderColor,
                                                        DS.Style.arrowRectangleBlur.darkBorderColor)
    StyledBehindWindowBlur {
        id: blur
        anchors.fill: parent
    }
}

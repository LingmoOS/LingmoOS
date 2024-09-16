// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

D.BehindWindowBlur {
    // maybe is a ApplicationWindow or Control
    property var control

    blendColor: {
        if (valid) {
            return DS.Style.control.selectColor(control ? control.palette.window :undefined,
                                        DS.Style.behindWindowBlur.lightColor,
                                        DS.Style.behindWindowBlur.darkColor)
        }
        return DS.Style.control.selectColor(control ? control.palette.window :undefined,
                                    DS.Style.behindWindowBlur.lightNoBlurColor,
                                    DS.Style.behindWindowBlur.darkNoBlurColor)
    }
}

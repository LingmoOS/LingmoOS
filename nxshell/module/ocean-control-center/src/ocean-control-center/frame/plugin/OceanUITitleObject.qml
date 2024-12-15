// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick.Controls 2.15

import org.lingmo.dtk 1.0 as D
import "OceanUIUtils.js" as OceanUIUtils

OceanUIObject {
    pageType: OceanUIObject.Item
    page: Label {
        font: OceanUIUtils.copyFont(D.DTK.fontManager.t4, {
                                    "bold": true
                                })
        text: oceanuiObj.displayName
    }
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.dtk 1.0 as DTK

BaseImageDelegate {
    id: delegate

    paintedPaddingWidth: (width - damagedIcon.width) / 2
    status: Image.Error

    DTK.DciIcon {
        id: damagedIcon

        anchors.centerIn: parent
        height: 151
        name: "photo_breach"
        theme: DTK.DTK.themeType
        width: 151
    }
}

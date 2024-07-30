// Copyright 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.19 as LingmoUI

QQC2.Button {
    id: root
    autoRepeat: true

    required property bool isEnd
    required property bool isStart

    readonly property color borderColor: if (enabled && (visualFocus || hovered || down)) {
        return LingmoUI.Theme.focusColor
    } else {
        return LingmoUI.ColorUtils.linearInterpolation(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, 0.15)
    }

    topPadding: 0
    bottomPadding: 0

    contentItem: Item {
        implicitHeight: LingmoUI.Units.gridUnit * 2
        LingmoUI.Icon {
            source: root.icon.name
            anchors.centerIn: parent
            implicitHeight: LingmoUI.Units.iconSizes.small
            implicitWidth: LingmoUI.Units.iconSizes.small
        }
    }

    background: LingmoUI.ShadowedRectangle {
        LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
        color: root.down ? LingmoUI.Theme.alternateBackgroundColor : LingmoUI.Theme.backgroundColor

        corners {
            topLeftRadius: root.isStart ? 4 : 0
            topRightRadius: root.isStart ? 4 : 0
            bottomLeftRadius: root.isEnd ? 4 : 0
            bottomRightRadius: root.isEnd ? 4 : 0
        }

        border {
            width: 1
            color: root.borderColor
        }
    }
}

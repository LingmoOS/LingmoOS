// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Templates as T
import org.deepin.dtk.style 1.0 as DS

T.PageIndicator {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentItem.implicitHeight + topPadding + bottomPadding)

    padding: DS.Style.control.padding
    spacing: DS.Style.control.padding

    delegate: Rectangle {
        implicitWidth: DS.Style.pageIndicator.width
        implicitHeight: DS.Style.pageIndicator.height

        radius: width / 2
        color: control.enabled ? control.palette.highlight : "gray"

        opacity: index === currentIndex ? 0.95 : pressed ? 0.7 : 0.45
        Behavior on opacity { OpacityAnimator { duration: 100 } }
    }

    contentItem: Row {
        spacing: control.spacing

        Repeater {
            model: control.count
            delegate: control.delegate
        }
    }
}

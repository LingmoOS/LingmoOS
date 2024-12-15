// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Templates 2.4 as T
import org.lingmo.dtk.impl 1.0 as D
import org.lingmo.dtk.style 1.0 as DS

T.ProgressBar {
    id: control

    property D.Palette backgroundColor: DS.Style.embeoceandProgressBar.background
    property D.Palette progressBackgroundColor: DS.Style.embeoceandProgressBar.progressBackground

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    padding: 1

    background: Rectangle {
        implicitWidth: DS.Style.embeoceandProgressBar.width
        implicitHeight: DS.Style.embeoceandProgressBar.height
        radius: DS.Style.embeoceandProgressBar.backgroundRadius
        color: control.D.ColorSelector.backgroundColor
    }

    contentItem: Item {
        Item {
            width: control.visualPosition * control.width
            height: DS.Style.embeoceandProgressBar.contentHeight
            clip: true

            Rectangle {
                color: control.D.ColorSelector.progressBackgroundColor
                radius: DS.Style.embeoceandProgressBar.contentRadius
                width: background.width
                height: parent.height
            }
        }
    }
}

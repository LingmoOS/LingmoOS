// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Templates as T
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

T.ProgressBar {
    id: control

    property D.Palette backgroundColor: DS.Style.embeddedProgressBar.background
    property D.Palette progressBackgroundColor: DS.Style.embeddedProgressBar.progressBackground

    implicitWidth: DS.Style.control.implicitWidth(control)
    implicitHeight: DS.Style.control.implicitHeight(control)
    padding: 1

    background: Rectangle {
        implicitWidth: DS.Style.embeddedProgressBar.width
        implicitHeight: DS.Style.embeddedProgressBar.height
        radius: DS.Style.embeddedProgressBar.backgroundRadius
        color: control.D.ColorSelector.backgroundColor
    }

    contentItem: Item {
        Item {
            width: control.visualPosition * control.width
            height: DS.Style.embeddedProgressBar.contentHeight
            clip: true

            Rectangle {
                color: control.D.ColorSelector.progressBackgroundColor
                radius: DS.Style.embeddedProgressBar.contentRadius
                width: background.width
                height: parent.height
            }
        }
    }
}

/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

import Lingmo.Accounts 1.0 as Accounts
import Lingmo.System 1.0 as System
import LingmoUI 1.0 as LingmoUI

ListView {
    id: control

    clip: true
    height: 80 + LingmoUI.Units.largeSpacing * 2
    orientation: ListView.Horizontal
    highlightRangeMode: ListView.StrictlyEnforceRange

    preferredHighlightBegin: width / 2 - userItemSize / 2
    preferredHighlightEnd: preferredHighlightBegin

    spacing: LingmoUI.Units.largeSpacing * 2

    property int userItemSize: 60

    delegate: Item {
        id: _item
        width: 60
        height: ListView.view.height

        opacity: control.currentIndex === index ? 1.0 : 0.3
        scale: control.currentIndex === index ? 1.0 : 0.9

        property string displayName: model.realName || model.name
        property string userName: model.name

        MouseArea {
            id: _itemMouseArea
            anchors.fill: parent
            onClicked: control.currentIndex = index
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: LingmoUI.Units.smallSpacing * 1.5

            Image {
                id: userIcon

                property int iconSize: 60

                Layout.preferredHeight: iconSize
                Layout.preferredWidth: iconSize
                sourceSize: String(model.icon).indexOf("/usr/share/sddm/faces/") > 0 ? Qt.size(iconSize, iconSize) : undefined
                source: model.icon
                Layout.alignment: Qt.AlignHCenter
                smooth: true

                scale: _itemMouseArea.pressed ? 0.9 : 1.0
                Behavior on scale {
                    NumberAnimation {
                        duration: 100
                    }
                }

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: userIcon.width
                        height: userIcon.height

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.height / 2
                        }
                    }
                }
            }

            QQC2.Label {
                Layout.alignment: Qt.AlignHCenter
                text: _item.displayName
            }
        }
    }
}

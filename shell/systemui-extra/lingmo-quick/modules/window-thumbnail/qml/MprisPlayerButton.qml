/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: qiqi <qiqi@kylinos.cn>
 *
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.lingmo.quick.items 1.0
import org.lingmo.quick.platform 1.0

StyleBackground {
    useStyleTransparency: false
    paletteRole: Theme.Base
    alpha: 0.65
    radius: 4
    border.width: 1
    borderColor: Theme.Text
    borderAlpha: 0.15

    property var playStatus
    signal skipBackwardClicked()
    signal playStatusClicked()
    signal skipForwardClicked()

    RowLayout {
        anchors.fill: parent
        spacing: 0
        MouseArea {
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            onClicked: skipBackwardClicked()

            Icon {
                height: 16
                width: 16
                anchors.centerIn: parent
                mode: Icon.AutoHighlight
                source: "media-skip-backward-symbolic"
            }
        }

        StyleBackground {
            useStyleTransparency: false
            Layout.preferredHeight: 16
            Layout.preferredWidth: 1
            border.width: 1
            borderColor: Theme.Text
            borderAlpha: 0.15
        }

        MouseArea {
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            onClicked: playStatusClicked()
            Icon {
                height: 16
                width: 16
                anchors.centerIn: parent
                mode: Icon.AutoHighlight
                source: playStatus === "Playing" ? "media-playback-pause-symbolic" : "lingmo-play-full-symbolic"
            }
        }

        StyleBackground {
            useStyleTransparency: false
            Layout.preferredHeight: 16
            Layout.preferredWidth: 1
            border.width: 1
            borderColor: Theme.Text
            borderAlpha: 0.15
        }

        MouseArea {
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            onClicked: skipForwardClicked()
            Icon {
                height: 16
                width: 16
                anchors.centerIn: parent
                mode: Icon.AutoHighlight
                source: "media-skip-forward-symbolic"
            }
        }
    }
}

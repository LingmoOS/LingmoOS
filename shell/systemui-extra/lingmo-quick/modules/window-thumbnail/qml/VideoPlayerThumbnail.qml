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
import org.lingmo.windowThumbnail 1.0

WindowThumbnail {
    id: windowThumbnail
    property var playerData
    property var viewModel

    BlurItem {
        x: buttonBase.x
        y: buttonBase.y
        width: buttonBase.width
        height: buttonBase.height
        radius: 4
        blurRadius: 40
        samples: 81
        source: windowThumbnail
    }

    MprisPlayerButton {
        id: buttonBase
        height: 28
        width: 128
        z: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        playStatus: model.PlaybackStatus

        onSkipBackwardClicked: {
            playerData.operation(viewModel.index(index, 0), MprisProperties.Previous, [])
        }
        onPlayStatusClicked: {
            playerData.operation(viewModel.index(index, 0), MprisProperties.PlayPause, [])
        }
        onSkipForwardClicked: {
            playerData.operation(viewModel.index(index, 0), MprisProperties.Next, [])
        }
    }
}

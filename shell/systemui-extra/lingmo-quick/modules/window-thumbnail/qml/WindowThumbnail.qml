/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
import org.lingmo.windowThumbnail 1.0

Item {
    id: thumbnailSourceItem
    clip: true

    property string winId
    property bool fixHeight: false
    property bool fixWidth: false

    Loader {
        id: thumbnailLoader
        height: parent.height
        width: parent.width
        visible: windowThumbnailConfig.realTimeThumbnailEnable
        sourceComponent: {
            if(windowThumbnailConfig.kywlcomWindowThumbnailEnable) {
                return kywlcomWindowThumbnailComonent;
            } else if (windowThumbnailConfig.pipewireThumbnailEnable) {
                return piperWireThumbnailComponent;
            } else if(windowThumbnailConfig.xThumbnailEnable) {
                return x11ThumbnailComponent;
            }
        }

        Component {
            id: x11ThumbnailComponent
            XWindowThumbnail {
                winId: Number.parseInt(thumbnailSourceItem.winId)
                fixHeight: thumbnailSourceItem.fixHeight
                fixWidth: thumbnailSourceItem.fixWidth
                width: parent.width
                height: parent.height
            }
        }
        Component {
            id: piperWireThumbnailComponent
            PipeWireThumbnail {
                fixHeight: thumbnailSourceItem.fixHeight
                fixWidth: thumbnailSourceItem.fixWidth
            }
        }
        Component {
            id: kywlcomWindowThumbnailComonent
            KywlcomWindowThumbnailItem {
                uuid: thumbnailSourceItem.winId
                fixHeight: thumbnailSourceItem.fixHeight
                fixWidth: thumbnailSourceItem.fixWidth
            }
        }
    }
}

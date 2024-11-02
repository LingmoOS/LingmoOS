/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.14
import org.lingmo.windowThumbnail 1.0
Item {
    property alias fixHeight: pipeWireSourceItem.fixHeight
    property alias fixWidth: pipeWireSourceItem.fixWidth
    width: fixHeight? pipeWireSourceItem.width : parent.width
    height: fixWidth? pipeWireSourceItem.height : parent.height
    PipeWireSourceItem {
        id: pipeWireSourceItem
        visible: waylandItem.nodeId > 0
        nodeId: waylandItem.nodeId
        width: fixHeight? paintedWidth : parent.width
        height: fixWidth? paintedHeight : parent.height
    }

    ScreenCastingRequest {
        id: waylandItem
        uuid: {thumbnailSourceItem.winId}
    }
}


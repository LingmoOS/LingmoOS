// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.ds.dock.tray 1.0 as DDT

Button {
    icon.name: model.surfaceId
    icon.width: 16
    icon.height: 16

    property size visualSize: Qt.size(width, height)

    width: model.surfaceId === "trash::trash" ? 36 : 16
    height: model.surfaceId === "trash::trash" ? 36 : 16

    Drag.active: dragHandler.active
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "text/x-dde-shell-tray-dnd-surfaceId": model.surfaceId
    }
    Drag.supportedActions: Qt.MoveAction
    Drag.onActiveChanged: {
        DDT.TraySortOrderModel.actionsAlwaysVisible = Drag.active
        if (!Drag.active) {
            // reset position on drop
            Qt.callLater(() => { x = 0; y = 0; });
        }
    }
    contentItem: Rectangle {
        color: "grey"
    }
    DragHandler {
        id: dragHandler
    }
}

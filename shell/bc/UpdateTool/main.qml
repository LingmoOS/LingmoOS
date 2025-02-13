/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
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
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import Lingmo.Updator 1.0

LingmoUI.Window {
    id: rootWindow
    width: 800
    height: 520
    minimumWidth: 720
    minimumHeight: 520
    maximumWidth: 720
    maximumHeight: 520
    visible: true
    title: qsTr("System update")

    minimizeButtonVisible: true

    flags: Qt.FramelessWindowHint

    property bool updating: false
    property bool updateSuccess: false

    onClosing: {
        // 关闭保护
        if (rootWindow.updating) {
            close.accepted = false
            return
        }

        close.accepted = true
    }

    // background.opacity: LingmoUI.Theme.darkMode ? 0.9 : 0.8

    LingmoUI.WindowBlur {
        view: rootWindow
        geometry: Qt.rect(rootWindow.x, rootWindow.y, rootWindow.width, rootWindow.height)
        windowRadius: rootWindow.windowRadius
        enabled: rootWindow.compositing
    }

    DragHandler {
        target: null
        acceptedDevices: PointerDevice.GenericPointer
        grabPermissions: PointerHandler.CanTakeOverFromItems
        onActiveChanged: if (active) { rootWindow.helper.startSystemMove(rootWindow) }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: homePage
    }

    Updator {
        id: updator

        onCheckUpdateFinished: {
            stackView.push(updatePage)
        }

        onStartingUpdate: {
            rootWindow.updating = true
            stackView.push(updatingPage)
        }

        onUpdateFinished: {
            rootWindow.updateSuccess = true
            rootWindow.updating = false
            stackView.push(finishedPage)
        }

        onUpdateError: {
            rootWindow.updateSuccess = false
            rootWindow.updating = false
            stackView.push(finishedPage)
        }
    }

    Component {
        id: homePage

        HomePage { }
    }

    Component {
        id: updatePage

        UpdatePage {}
    }

    Component {
        id: updatingPage

        UpdatingPage {}
    }

    Component {
        id: finishedPage

        FinishedPage {}
    }
}

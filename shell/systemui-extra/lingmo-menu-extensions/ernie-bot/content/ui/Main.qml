/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.lingmo.menu.extension 1.0
import QtQuick.Controls 2.12

/**
 *     property var extensionData;
 *     signal send(var data);
 */

LingmoUIMenuExtension {
    id: root

    Component.onCompleted: {
        extensionData.sessionManager.checkApiReady();
    }

    Loader {
        anchors.fill: parent
        property var mainPage: root
        property var extensionData: root.extensionData
        property bool apiReady: root.extensionData.sessionManager.apiReady

        sourceComponent: apiReady ? sessionPage : sessionLoginPage
    }

    Component {
        id: sessionLoginPage
        SessionLoginPage {}
    }

    Component {
        id: sessionPage
        SessionPage {}
    }
}

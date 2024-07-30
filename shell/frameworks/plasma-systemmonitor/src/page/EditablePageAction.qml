/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.page

Kirigami.PagePoolAction {
    id: action

    property PageDataObject pageData

    page: Qt.resolvedUrl("EditablePage.qml") + "?page=" + pageData.fileName

    initialProperties: {
        "pageData": action.pageData
    }

    Component.onCompleted: {
        if (pageData.loadType == "onstart") {
            pagePool.loadPageWithProperties(page, initialProperties)
        }
    }
}

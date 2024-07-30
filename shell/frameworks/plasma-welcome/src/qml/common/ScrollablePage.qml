/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

GenericPage {
    id: page

    property alias view: scroll.contentItem

    // Content is expected to bring its own
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    QQC2.ScrollView {
        id: scroll
        anchors.fill: parent

        QQC2.ScrollBar.horizontal.visible: false
    }

    Kirigami.Separator {
        width: parent.width
        anchors.bottom: parent.top
    }
}

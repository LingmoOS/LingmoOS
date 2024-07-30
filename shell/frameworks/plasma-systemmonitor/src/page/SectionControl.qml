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

Container {
    id: control

    property PageDataObject sectionData

    signal addSection()
    signal addSeparator()

    function replaceSensors(replacement) {
        contentItem.replaceSensors(replacement)
    }

    implicitWidth: (sectionData.isSeparator ? toolbar.Layout.minimumWidth : 0) + leftPadding + rightPadding

    Kirigami.Separator {
        anchors {
            top: parent.top
            topMargin: parent.topPadding
            bottom: parent.bottom
            bottomMargin: parent.bottomPadding
            horizontalCenter: parent.horizontalCenter
        }
        visible: modelData.isSeparator
    }

    contentItem: FaceControl {
        anchors {
            fill: parent
            topMargin: parent.topPadding
            bottomMargin: parent.bottomPadding
            leftMargin: parent.leftPadding
            rightMargin: parent.rightPadding
        }
        activeItem: control.activeItem
        visible: !modelData.isSeparator
        dataObject: control.sectionData

        onSelect: item => control.select(item)

        onMissingSensorsChanged: (id, title, sensors) => control.missingSensorsChanged(id, title, sensors)
    }

    toolbar.addVisible: false
    toolbar.moveAxis: Qt.XAxis
}

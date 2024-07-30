/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import org.kde.kirigami as Kirigami
import QtQuick.Templates as T
import org.kde.qqc2desktopstyle.private as StylePrivate

T.TabBar {
    id: controlRoot

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    //Some QStyles seem to not return sensible pixelmetrics here
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding,
                            Kirigami.Units.gridUnit * 6)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: 0

    contentItem: ListView {
        implicitWidth: contentWidth
        // The binding to contentModel.count is such that it updates when the TabBar is populated on demand
        implicitHeight: controlRoot.contentModel.get(controlRoot.contentModel.count * 0).height

        model: controlRoot.contentModel
        currentIndex: controlRoot.currentIndex

        spacing: -styleItem.pixelMetric("tabOverlap")
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

        highlightMoveDuration: 0
        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: 40
        preferredHighlightEnd: width - 40
    }

    StylePrivate.StyleItem {
        id: styleItem
        control: controlRoot
        visible: false
        elementType: "tabframe"
        properties: {
            "orientation": controlRoot.position === T.TabBar.Header ? "Top" : "Bottom"
        }
    }

    background: MouseArea {
        acceptedButtons: Qt.NoButton
        onWheel: {
            if (wheel.pixelDelta.y < 0 || wheel.angleDelta.y < 0) {
                controlRoot.currentIndex = Math.min(controlRoot.currentIndex + 1, controlRoot.contentModel.count - 1);
            } else {
                controlRoot.currentIndex = Math.max(controlRoot.currentIndex - 1, 0);
            }
        }
    }
}

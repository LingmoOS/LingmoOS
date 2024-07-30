/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Item {
    id: root
    implicitWidth: control.implicitWidth
    implicitHeight: control.implicitHeight
    property bool raised: false
    property bool single: true
    property bool alwaysShowBackground: false

    property Item activeItem
    readonly property bool active: activeItem == control || activeItem == root

    property int index

    signal select(Item item)
    signal remove()
    signal move(int from, int to)

    signal missingSensorsChanged(string id, string title, var sensors)

    property alias hovered: control.hovered
    property alias contentItem: control.contentItem
    property alias background: control.background
    property alias mainControl: control
    property alias leftPadding: control.leftPadding
    property alias rightPadding: control.rightPadding
    property alias topPadding: control.topPadding
    property alias bottomPadding: control.bottomPadding

    readonly property alias toolbar: toolbar

    // We distinguish between two different minimum heights: the minimum height
    // of the contents and the minimum height of the entire item. The content
    // height should match the minimum height of the innermost item, usually
    // this will be a face. The maximum of the content heights is then used for
    // all rows to ensure rows have the same height. The minimum height is used
    // for the actual layouting and includes space for things like the toolbar.

    property real minimumContentHeight: contentItem && contentItem.hasOwnProperty("minimumContentHeight") ?
                                            contentItem.minimumContentHeight : 0
    property real minimumHeight: contentItem && contentItem.hasOwnProperty("minimumHeight") ?
                                            contentItem.minimumHeight + topPadding + bottomPadding : 0

    z: raised ? 99 : 0

    Control {
        id: control

        hoverEnabled: true

        width: root.width
        height: root.height

        leftPadding: Kirigami.Units.largeSpacing
        Behavior on leftPadding { NumberAnimation { duration: Kirigami.Units.shortDuration } }
        rightPadding: Kirigami.Units.largeSpacing
        Behavior on rightPadding { NumberAnimation { duration: Kirigami.Units.shortDuration } }
        bottomPadding: Kirigami.Units.largeSpacing
        Behavior on bottomPadding { NumberAnimation { duration: Kirigami.Units.shortDuration } }
        topPadding: root.active ? toolbar.height + Kirigami.Units.smallSpacing * 2 : Kirigami.Units.largeSpacing * 2
        Behavior on topPadding { NumberAnimation { duration: Kirigami.Units.shortDuration } }

        background: Item {
            PlaceholderRectangle {
                id: backgroundRectangle
                anchors.fill: parent
                highlight: root.hovered
                opacity: root.alwaysShowBackground || root.hovered ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                onClicked: root.select(control)

                shadow.size: root.raised ? Kirigami.Units.gridUnit : 0
            }

            EditorToolBar {
                id: toolbar

                z: 1

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    topMargin: Kirigami.Units.smallSpacing
                    leftMargin: Kirigami.Units.largeSpacing
                    rightMargin: Kirigami.Units.largeSpacing
                }

                single: root.single
                moveTarget: root

                enabled: opacity >= 1
                opacity: root.active ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }

                onMoved: root.move(from, to)
                onRemoveClicked: root.remove()
            }
        }
    }
}

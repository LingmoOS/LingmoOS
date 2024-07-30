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

    property PageDataObject rowData
    property Page page

    property string heightMode: {
        if (rowData.heightMode) {
            print("heightMode", rowData.heightMode)
            return rowData.heightMode
        }

        if (rowData.isTitle) {
            return "minimum"
        }

        return "balanced"
    }

    implicitHeight: heading.height + topPadding + bottomPadding

    alwaysShowBackground: true

    function updateMinimumHeight() {
        if (rowData.isTitle) {
            minimumContentHeight = heading.implicitHeight
            minimumHeight = implicitHeight
            return
        }

        let minHeight = 0;
        let minContentHeight = 0;
        for (let i = 0; i < repeater.count; ++i) {
            let item = repeater.itemAt(i)
            if (item) {
                minContentHeight = Math.max(minContentHeight, item.minimumContentHeight)
                minHeight = Math.max(minHeight, item.minimumHeight)
            }
        }

        minimumContentHeight = minContentHeight
        minimumHeight = minHeight + control.topPadding + control.bottomPadding
    }

    function replaceSensors(replacement) {
        for (let i = 0; i < repeater.count; ++i) {
            repeater.itemAt(i).replaceSensors(replacement)
        }
    }

    onTopPaddingChanged: Qt.callLater(updateMinimumHeight)
    onBottomPaddingChanged: Qt.callLater(updateMinimumHeight)

    contentItem: Row {
        anchors.fill: parent
        anchors.topMargin: control.topPadding
        anchors.leftMargin: control.leftPadding
        anchors.rightMargin: control.rightPadding
        anchors.bottomMargin: control.bottomPadding

        spacing: Kirigami.Units.largeSpacing

        move: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
        }

        Kirigami.Heading {
            id: heading
            width: parent.width
            text: control.rowData.title ? control.rowData.title : ""
            visible: control.rowData.isTitle && !control.active
            level: 2

            onHeightChanged: Qt.callLater(control.updateMinimumHeight)
        }

        TextField {
            width: parent.width
            text: control.rowData.title ? control.rowData.title : ""
            visible: control.rowData.isTitle && control.active
            onTextEdited: control.rowData.title = text
        }

        Repeater {
            id: repeater
            model: PageDataModel { data: control.rowData }

            onItemAdded: Qt.callLater(control.updateMinimumHeight)
            onItemRemoved: Qt.callLater(control.updateMinimumHeight)

            ColumnControl {
                height: parent.height
                width: (parent.width - parent.spacing * (repeater.count - 1)) / repeater.count

                onMinimumContentHeightChanged: Qt.callLater(control.updateMinimumHeight)
                onMinimumHeightChanged: Qt.callLater(control.updateMinimumHeight)

                activeItem: control.activeItem
                single: control.rowData.children.length == 1
                columnData: model.data
                index: model.index

                onSelect: item => control.select(item)
                onRemove: control.rowData.removeChild(index)
                onMove: (from, to) => control.rowData.moveChild(from, to)

                onMissingSensorsChanged: (id, title, sensors) => control.missingSensorsChanged(id, title, sensors)
            }
        }
    }

    toolbar.addActions: [
        Kirigami.Action {
            text: i18nc("@action", "Add Column")
            onTriggered: control.addColumn(control.rowData.children.length)
        }
    ]
    toolbar.addVisible: !control.rowData.isTitle
    toolbar.page: page

    toolbar.extraActions: [
        Kirigami.Action {
            text: i18nc("@action", "Height")
            visible: !control.rowData.isTitle

            Kirigami.Action {
                text: i18nc("@action:inmenu", "Balanced")
                tooltip: i18nc("@info:tooltip", "Try and use the same height across different rows.")
                ActionGroup.group: heightGroup
                checkable: true
                checked: control.rowData.heightMode == "balanced"
                onTriggered: control.rowData.heightMode = "balanced"
            }

            Kirigami.Action {
                text: i18nc("@action:inmenu", "Minimum")
                tooltip: i18nc("@info:tooltop", "Use the minimum amount of height needed to display contents correctly.")
                ActionGroup.group: heightGroup
                checkable: true
                checked: control.rowData.heightMode == "minimum"
                onTriggered: control.rowData.heightMode = "minimum"
            }

            Kirigami.Action {
                text: i18nc("@action:inmenu", "Maximum")
                tooltip: i18nc("@info:tooltip", "Try to use as much height as possible.")
                ActionGroup.group: heightGroup
                checkable: true
                checked: control.rowData.heightMode == "maximum"
                onTriggered: control.rowData.heightMode = "maximum"
            }
        }
    ]

    ActionGroup { id: heightGroup }

    function addColumn(index) {
        control.rowData.insertChild(index, {"name": "column-" + index, showBackground: true})
    }

    Component.onCompleted: {
        if (rowData.children.length == 0 && !rowData.isTitle) {
            addColumn(0)
        }
    }
}

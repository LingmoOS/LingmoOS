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

Column {
    id: root

    property Page parentPage
    property PageDataObject pageData
    property Item activeItem
    property var actionsFace

    property var missingSensors: []
    signal showMissingSensors()

    spacing: rowSpacing // From parent loader

    move: Transition {
        NumberAnimation { properties: "x,y"; duration: Kirigami.Units.shortDuration }
    }

    function relayout() {
        let reservedHeight = missingMessage.visible ? missingMessage.height : 0;
        let minimumHeight = 0;
        let balancedCount = 0;
        let maximumCount = 0;

        for (let i = 0; i < repeater.count; ++i) {
            let child = repeater.itemAt(i)
            if (!child || !child.hasOwnProperty("heightMode")) {
                // Apparently not a RowControl, ignore it.
                continue;
            }

            switch(child.heightMode) {
                case "minimum":
                    reservedHeight += child.minimumHeight
                    break;
                case "balanced":
                    minimumHeight = Math.max(child.minimumHeight, minimumHeight)
                    balancedCount += 1
                    break;
                case "maximum":
                    maximumCount += 1
                    break;
            }
        }

        let layoutHeight = availableHeight - reservedHeight - root.spacing * (repeater.count - 1)

        let balancedHeight = balancedCount > 0 ? layoutHeight / balancedCount : 0
        let maximumHeight = maximumCount > 0 ? (layoutHeight - minimumHeight * balancedCount) / maximumCount : 0

        let minimumTotalHeight = 0

        for (let i = 0; i < repeater.count; ++i) {
            let child = repeater.itemAt(i)
            if (!child || !child.hasOwnProperty("heightMode")) {
                continue;
            }

            minimumTotalHeight += root.spacing;

            switch (child.heightMode) {
                case "minimum":
                    child.height = child.minimumHeight
                    break
                case "balanced":
                    if (maximumCount > 0) {
                        child.height = child.minimumHeight
                    } else {
                        child.height = Math.max(minimumHeight, balancedHeight)
                    }
                    break
                case "maximum":
                    // The last "balanced" here is to make sure if the user
                    // selects "maximum" it is visually distinct from "balanced".
                    child.height = Math.max(child.minimumHeight, maximumHeight, balancedHeight * 1.25)
                    break
            }

            minimumTotalHeight += child.height
        }

        Layout.minimumHeight = minimumTotalHeight
    }

    function updateMissingSensors(id, title, sensors) {
        for (let sensor of sensors) {
            missingSensors.push({
                "face": id,
                "title": title,
                "sensor": sensor
            })
        }
        missingSensorsChanged()
    }

    function replaceSensors(replacement) {
        if (!replacement) {
            return
        }

        missingSensors = []

        for (let i = 0; i < repeater.count; ++i) {
            repeater.itemAt(i).replaceSensors(replacement)
        }
    }

    onWidthChanged: Qt.callLater(relayout)
    onHeightChanged: Qt.callLater(relayout)

    Kirigami.InlineMessage {
        id: missingMessage

        width: parent.width

        visible: root.missingSensors.length > 0
        type: Kirigami.MessageType.Error
        text: i18n("This page is missing some sensors and will not display correctly.");

        actions: Kirigami.Action {
            icon.name: "document-edit"
            text: i18nc("@action:button", "Fix…")
            onTriggered: root.showMissingSensors()
        }
    }

    Repeater {
        id: repeater
        model: PageDataModel { data: root.pageData }

        onItemAdded: Qt.callLater(root.relayout)
        onItemRemoved: Qt.callLater(root.relayout)

        RowControl {
            width: parent.width

            onHeightChanged: Qt.callLater(root.relayout)
            onMinimumContentHeightChanged: Qt.callLater(root.relayout)
            onMinimumHeightChanged: Qt.callLater(root.relayout)
            onHeightModeChanged: Qt.callLater(root.relayout)

            rowData: model.data

            activeItem: root.activeItem
            single: root.pageData.children.length == 1

            index: model.index

            page: root.parentPage

            onSelect: item => root.activeItem = item

            onRemove: pageData.removeChild(index)
            onMove: (from, to) => pageData.moveChild(from, to)

            onMissingSensorsChanged: (id, title, sensors) => root.updateMissingSensors(id, title, sensors)
        }
    }

    function addTitle(index) {
        if (index < 0) {
            index = pageData.children.length
        }

        pageData.insertChild(index, {
            name: "row-" + index,
            isTitle: true,
            title: i18n("New Title"),
            heightMode: "minimum"
        })
    }

    function addRow(index) {
        if (index < 0) {
            index = pageData.children.length
        }

        pageData.insertChild(index, {
            name: "row-" + index,
            isTitle: false,
            title: "",
            heightMode: "balanced"
        })
    }

    Component.onCompleted: {
        if (pageData.children.length == 0) {
            addRow(0)
        }
    }
}

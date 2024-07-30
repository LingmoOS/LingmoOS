/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.ksysguard.sensors as Sensors

Control {
    id: control

    property bool supportsColors: true
    property bool labelsEditable: true
    property int maxAllowedSensors: -1
    property var selected: []
    property var colors: {}
    property var labels: {}

    signal selectColor(string sensorId)
    signal colorForSensorGenerated(string sensorId, color color)
    signal sensorLabelChanged(string sensorId, string label)

    onSelectedChanged: {
        if (!control.selected) {
            return;
        }
        for (let i = 0; i < Math.min(control.selected.length, selectedModel.count); ++i) {
            selectedModel.set(i, {"sensor": control.selected[i]});
        }
        if (selectedModel.count > control.selected.length) {
            selectedModel.remove(control.selected.length, selectedModel.count - control.selected.length);
        } else if (selectedModel.count < control.selected.length) {
            for (let i = selectedModel.count; i < control.selected.length; ++i) {
                selectedModel.append({"sensor": control.selected[i]});
            }
        }
    }

    background: TextField {
        readOnly: true
        hoverEnabled: false

        placeholderText: control.selected.length == 0 ? i18ndc("KSysGuardSensorFaces", "@label", "Click to select a sensor…") : ""

        onFocusChanged: {
            if (focus && (maxAllowedSensors <= 0 || repeater.count < maxAllowedSensors)) {
                popup.open()
            } else {
                popup.close()
            }
        }
        onReleased: {
            if (focus && (maxAllowedSensors <= 0 || repeater.count < maxAllowedSensors)) {
                popup.open()
            }
        }
    }

    contentItem: Flow {
        spacing: Kirigami.Units.smallSpacing

        move: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
        Repeater {
            id: repeater
            model: ListModel {
                id: selectedModel
                function writeSelectedSensors() {
                    let newSelected = [];
                    for (let i = 0; i < count; ++i) {
                        newSelected.push(get(i).sensor);
                    }
                    control.selected = newSelected;
                    control.selectedChanged();
                }
            }

            delegate: Item {
                id: delegate
                implicitHeight: layout.implicitHeight + Kirigami.Units.smallSpacing * 2
                implicitWidth: Math.min(layout.implicitWidth + Kirigami.Units.smallSpacing * 2,
                                        control.width - control.leftPadding - control.rightPadding)
                readonly property int position: index
                Rectangle {
                    id: delegateContents
                    z: 10
                    color: Qt.rgba(
                                Kirigami.Theme.highlightColor.r,
                                Kirigami.Theme.highlightColor.g,
                                Kirigami.Theme.highlightColor.b,
                                0.25)
                    radius: Kirigami.Units.smallSpacing
                    border.color: Kirigami.Theme.highlightColor
                    border.width: 1
                    opacity: (control.maxAllowedSensors <= 0 || index < control.maxAllowedSensors) ? 1 : 0.4
                    parent: drag.active ? control : delegate

                    width: delegate.width
                    height: delegate.height
                    DragHandler {
                        id: drag
                        //TODO: uncomment as soon as we can depend from 5.15
                        cursorShape: active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        enabled: selectedModel.count > 1
                        onActiveChanged: {
                            if (active) {
                                let pos = delegateContents.mapFromItem(control.contentItem, 0, 0);
                                delegateContents.x = pos.x;
                                delegateContents.y = pos.y;
                            } else {
                                let pos = delegate.mapFromItem(delegateContents, 0, 0);
                                delegateContents.x = pos.x;
                                delegateContents.y = pos.y;
                                dropAnim.restart();
                                selectedModel.writeSelectedSensors();
                            }
                        }
                        xAxis {
                            minimum: 0
                            maximum: control.width - delegateContents.width
                        }
                        yAxis {
                            minimum: 0
                            maximum: control.height - delegateContents.height
                        }
                        onCentroidChanged: {
                            if (!active || control.contentItem.move.running) {
                                return;
                            }
                            let pos = control.contentItem.mapFromItem(null, drag.centroid.scenePosition.x, drag.centroid.scenePosition.y);
                            pos.x = Math.max(0, Math.min(control.contentItem.width - 1, pos.x));
                            pos.y = Math.max(0, Math.min(control.contentItem.height - 1, pos.y));

                            let child = control.contentItem.childAt(pos.x, pos.y);
                            if (child === delegate) {
                                return;
                            } else if (child) {
                                let newIndex = -1;
                                if (pos.x > child.x + child.width/2) {
                                    newIndex = Math.min(child.position + 1, selectedModel.count - 1);
                                } else {
                                    newIndex = child.position;
                                }
                                selectedModel.move(index, newIndex, 1);
                            }
                        }
                    }
                    ParallelAnimation {
                        id: dropAnim
                        XAnimator {
                            target: delegateContents
                            from: delegateContents.x
                            to: 0
                            duration: Kirigami.Units.shortDuration
                            easing.type: Easing.InOutQuad
                        }
                        YAnimator {
                            target: delegateContents
                            from: delegateContents.y
                            to: 0
                            duration: Kirigami.Units.shortDuration
                            easing.type: Easing.InOutQuad
                        }
                    }

                    Sensors.Sensor { id: sensor; sensorId: model.sensor }

                    Component.onCompleted: {
                        if (typeof control.colors === "undefined" ||
                            typeof control.colors[sensor.sensorId] === "undefined") {
                            let color = Qt.hsva(Math.random(), Kirigami.Theme.highlightColor.hsvSaturation, Kirigami.Theme.highlightColor.hsvValue, 1);
                            control.colorForSensorGenerated(sensor.sensorId, color)
                        }
                    }

                    RowLayout {
                        id: layout

                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.smallSpacing

                        ToolButton {
                            visible: control.supportsColors
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium

                            padding: Kirigami.Units.smallSpacing
                            flat: false

                            contentItem: Rectangle {
                                color: typeof control.colors === "undefined"  ? "black" : control.colors[sensor.sensorId]
                            }

                            onClicked: control.selectColor(sensor.sensorId)
                        }

                        RowLayout {
                            id: normalLayout
                            Label {
                                id: label
                                Layout.fillWidth: true
                                text: {
                                    if (!control.labels || !control.labels[sensor.sensorId]) {
                                        return sensor.name
                                    }
                                    return control.labels[sensor.sensorId]
                                }
                                elide: Text.ElideRight

                                HoverHandler { id: handler }

                                ToolTip.text: sensor.name
                                ToolTip.visible: handler.hovered && label.truncated
                                ToolTip.delay: Kirigami.Units.toolTipDelay
                            }
                            ToolButton {
                                id: editButton
                                visible: control.labelsEditable
                                icon.name: "document-edit"
                                icon.width: Kirigami.Units.iconSizes.small
                                icon.height: Kirigami.Units.iconSizes.small
                                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                                onClicked: layout.state = "editing"
                            }
                            ToolButton {
                                id: removeButton
                                icon.name: "edit-delete-remove"
                                icon.width: Kirigami.Units.iconSizes.small
                                icon.height: Kirigami.Units.iconSizes.small
                                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium

                                onClicked: {
                                    if (control.selected === undefined || control.selected === null) {
                                        control.selected = []
                                    }
                                    control.selected.splice(control.selected.indexOf(sensor.sensorId), 1)
                                    control.selectedChanged()
                                }
                            }
                        }

                        Loader {
                            id: editLoader
                            active: false
                            visible: active
                            focus: active
                            Layout.fillWidth: true
                            sourceComponent: RowLayout {
                                id: editLayout
                                TextField {
                                    id: textField
                                    Layout.fillWidth: true
                                    text: label.text
                                    cursorPosition: 0
                                    focus: true
                                    onAccepted: {
                                        if (text == sensor.name) {
                                            text = ""
                                        }
                                        sensorLabelChanged(sensor.sensorId, text)
                                        layout.state = ""
                                    }
                                }
                                ToolButton {
                                    icon.name: "checkmark"
                                    width: Kirigami.Units.iconSizes.smallMedium
                                    Layout.preferredHeight: textField.implicitHeight
                                    Layout.preferredWidth: Layout.preferredHeight
                                    onClicked: textField.accepted()
                                }
                            }
                        }

                        states: State {
                            name: "editing"
                            PropertyChanges {
                                target: normalLayout
                                visible: false
                            }
                            PropertyChanges {
                                target: editLoader
                                active: true
                            }
                            PropertyChanges {
                                target: delegate
                                implicitWidth: control.availableWidth
                            }
                        }
                        transitions: Transition {
                            PropertyAnimation {
                                target: delegate
                                properties: "implicitWidth"
                                duration: Kirigami.Units.shortDuration
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
            }
        }

        Item {
            width: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing * 2
            height: width
            visible: control.maxAllowedSensors <= 0 || control.selected.length < control.maxAllowedSensors
        }
    }

    Popup {
        id: popup

        // Those bindings will be immediately broken on show, but they're needed to not show the popup at a wrong position for an instant
        y: (control.Kirigami.ScenePosition.y + control.height + height > control.Window.height)
            ? - height
            : control.height
        implicitHeight: Math.min(contentItem.implicitHeight + 2, Kirigami.Units.gridUnit * 20)
        width: control.width + 2
        topMargin: 6
        bottomMargin: 6
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        padding: 1

        onOpened: {
            if (control.Kirigami.ScenePosition.y + control.height + height > control.Window.height) {
                y = - height;
            } else {
                y = control.height
            }

            searchField.forceActiveFocus();
        }
        onClosed: delegateModel.rootIndex = delegateModel.parentModelIndex()

        contentItem: ColumnLayout {
            spacing: 0
            ToolBar {
                Layout.fillWidth: true
                Layout.minimumHeight: implicitHeight
                Layout.maximumHeight: implicitHeight
                contentItem: ColumnLayout {

                    Kirigami.SearchField {
                        id: searchField
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        placeholderText: i18nd("KSysGuardSensorFaces", "Search...")
                        onTextEdited: listView.searchString = text
                        onAccepted: listView.searchString = text
                        KeyNavigation.down: listView
                    }

                    RowLayout {
                        visible: delegateModel.rootIndex.valid
                        Layout.maximumHeight: visible ? implicitHeight : 0
                        ToolButton {
                            Layout.fillHeight: true
                            Layout.preferredWidth: height
                            icon.name: "go-previous"
                            text: i18ndc("KSysGuardSensorFaces", "@action:button", "Back")
                            display: Button.IconOnly
                            onClicked: delegateModel.rootIndex = delegateModel.parentModelIndex()
                        }
                        Kirigami.Heading {
                            level: 2
                            text: delegateModel.rootIndex.model ? delegateModel.rootIndex.model.data(delegateModel.rootIndex) : ""
                        }
                    }
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                clip: true

                ListView {
                    id: listView

                    // this causes us to load at least one delegate
                    // this is essential in guessing the contentHeight
                    // which is needed to initially resize the popup
                    cacheBuffer: 1

                    property string searchString

                    implicitHeight: contentHeight

                    model: DelegateModel {
                        id: delegateModel

                        model: listView.searchString ? sensorsSearchableModel : treeModel
                        delegate: ItemDelegate {
                            id: listItem

                            width: listView.width

                            text: model.display

                            leftPadding: mirrored ? indicator.implicitWidth + Kirigami.Units.largeSpacing * 2 : Kirigami.Units.largeSpacing
                            rightPadding: !mirrored ? indicator.implicitWidth + Kirigami.Units.largeSpacing * 2 : Kirigami.Units.largeSpacing

                            indicator: Kirigami.Icon {
                                anchors.right: parent.right
                                anchors.rightMargin: Kirigami.Units.largeSpacing
                                anchors.verticalCenter: parent.verticalCenter

                                width: Kirigami.Units.iconSizes.small
                                height: width
                                source: "go-next-symbolic"
                                opacity: model.SensorId.length == 0
                            }

                            onClicked: {
                                if (model.SensorId.length == 0) {
                                    delegateModel.rootIndex = delegateModel.modelIndex(index);
                                } else {
                                    if (control.selected === undefined || control.selected === null) {
                                        control.selected = []
                                    }
                                    const length = control.selected.push(model.SensorId)
                                    control.selectedChanged()
                                    if (control.maxAllowedSensors == length) {
                                        popup.close();
                                    }
                                }
                            }
                        }
                    }

                    Sensors.SensorTreeModel { id: treeModel }

                    KItemModels.KSortFilterProxyModel {
                        id: sensorsSearchableModel
                        filterCaseSensitivity: Qt.CaseInsensitive
                        filterString: listView.searchString
                        sourceModel: KItemModels.KSortFilterProxyModel {
                            filterRowCallback: function(row, parent) {
                                var sensorId = sourceModel.data(sourceModel.index(row, 0), Sensors.SensorTreeModel.SensorId)
                                return sensorId.length > 0
                            }
                            sourceModel: KItemModels.KDescendantsProxyModel {
                                model: listView.searchString ? treeModel : null
                            }
                        }
                    }

                    highlightRangeMode: ListView.ApplyRange
                    highlightMoveDuration: 0
                    boundsBehavior: Flickable.StopAtBounds
                }
            }
        }

        background: Item {
            anchors {
                fill: parent
                margins: -1
            }

            Kirigami.ShadowedRectangle {
                anchors.fill: parent
                anchors.margins: 1

                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false

                radius: 2
                color: Kirigami.Theme.backgroundColor

                property color borderColor: Kirigami.Theme.textColor
                border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
                border.width: 1

                shadow.xOffset: 0
                shadow.yOffset: 2
                shadow.color: Qt.rgba(0, 0, 0, 0.3)
                shadow.size: 8
            }
        }
    }
}

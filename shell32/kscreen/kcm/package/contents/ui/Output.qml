/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

Item {
    id: output

    readonly property bool isSelected: root.selectedOutput === model.index
    property size outputSize: model.size

    onIsSelectedChanged: {
        if (isSelected) {
            z = 89;
        } else {
            z = 0;
        }
    }

    function getAbsolutePosition(pos) {
        return Qt.point((pos.x - screen.xOffset) * screen.relativeFactor,
                        (pos.y - screen.yOffset) * screen.relativeFactor);
    }

    visible: model.enabled && model.replicationSourceIndex === 0

    onVisibleChanged: screen.resetTotalSize()
    onOutputSizeChanged: screen.resetTotalSize()

    x: model.position ? model.position.x / screen.relativeFactor + screen.xOffset : 0
    y: model.position ? model.position.y / screen.relativeFactor + screen.yOffset : 0

    width: model.size ? model.size.width / screen.relativeFactor : 1
    height: model.size ? model.size.height / screen.relativeFactor : 1

    Rectangle {
        id: outline

        readonly property int orientationPanelWidth: 10
        readonly property real orientationPanelPosition: 1 - (orientationPanelWidth / outline.height)

        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        radius: Kirigami.Units.smallSpacing

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Kirigami.Theme.alternateBackgroundColor
            }
            GradientStop {
                // Create a hard cut. Can't use the same number otherwise it gets confused.
                position: outline.orientationPanelPosition - Number.EPSILON
                color: Kirigami.Theme.alternateBackgroundColor
            }
            GradientStop {
                position: outline.orientationPanelPosition
                color: outline.border.color
            }
            GradientStop {
                position: 1.0
                color: outline.border.color
            }
        }

        border {
            color: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
            width: 1

            Behavior on color {
                PropertyAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }
        }
    }

    Item {
        anchors.fill: parent
        // So we can show a grabby hand cursor when hovered over
        HoverHandler {
            cursorShape: kcm.outputModel && kcm.outputModel.rowCount() > 1 ? Qt.SizeAllCursor : undefined
        }
        z: 2
    }

    Item {
        id: labelContainer
        anchors {
            fill: parent
            margins: outline.border.width
        }

        // so the text is drawn above orientationPanelContainer
        z: 1
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 0
            width: parent.width
            Layout.maximumHeight: parent.height

            QQC2.Label {
                Layout.fillWidth: true
                Layout.maximumHeight: labelContainer.height - resolutionLabel.implicitHeight

                text: model.display
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            QQC2.Label {
                id: resolutionLabel
                Layout.fillWidth: true

                text: "(" + model.resolution.width + "x" + model.resolution.height +
                      (model.scale !== 1 ? "@" + Math.round(model.scale * 100.0) + "%": "") + ")"
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }
        }
    }

    states: [
        State {
            name: "transposed"
            PropertyChanges {
                target: outline
                width: output.height
                height: output.width
            }
        },

        State {
            name: "rot0"
            when: model.rotation === 1
            PropertyChanges {
                target: labelContainer
                anchors.bottomMargin: outline.orientationPanelWidth + outline.border.width
            }
        },
        State {
            name: "rot90"
            extend: "transposed"
            when: model.rotation === 2
            PropertyChanges {
                target: outline
                rotation: 90
            }
            PropertyChanges {
                target: labelContainer
                anchors.leftMargin: outline.orientationPanelWidth + outline.border.width
            }
        },
        State {
            name: "rot180"
            when: model.rotation === 4
            PropertyChanges {
                target: outline
                rotation: 180
            }
            PropertyChanges {
                target: labelContainer
                anchors.topMargin: outline.orientationPanelWidth + outline.border.width
            }
        },
        State {
            name: "rot270"
            extend: "transposed"
            when: model.rotation === 8
            PropertyChanges {
                target: outline
                rotation: 270
            }
            PropertyChanges {
                target: labelContainer
                anchors.rightMargin: outline.orientationPanelWidth + outline.border.width
            }
        }
    ]

    Rectangle {
        id: posLabel

        y: 4
        x: 4
        width: childrenRect.width + 5
        height: childrenRect.height + 2
        radius: 4

        opacity: model.enabled &&
                 (tapHandler.isLongPressed || dragHandler.active) ? 0.9 : 0.0


        color: Kirigami.Theme.disabledTextColor

        Text {
            id: posLabelText

            y: 2
            x: 2

            text: model.normalizedPosition.x + "," + model.normalizedPosition.y
            color: "white"
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: Kirigami.Units.longDuration
            }
        }
    }

    QQC2.ToolButton {
        id: replicas

        property int selectedReplica: -1

        height: output.height / 4
        width: output.width / 5
        anchors.top: output.top
        anchors.right: output.right
        anchors.margins: 5

        visible: model.replicasModel.length > 0
        icon.name: "osd-duplicate"

        QQC2.ToolTip {
            text: i18n("Replicas")
        }

        onClicked: {
            var index = selectedReplica + 1;
            if (index >= model.replicasModel.length) {
                index = 0;
            }
            if (root.selectedOutput !== model.replicasModel[index]) {
                root.selectedOutput = model.replicasModel[index];
            }
        }
    }

    property point dragStartPosition

    TapHandler {
        id: tapHandler
        property bool isLongPressed: false
        gesturePolicy: TapHandler.WithinBounds

        onPressedChanged: {
            if (pressed) {
                root.selectedOutput = model.index;
                dragStartPosition = Qt.point(output.x, output.y)
            } else {
                isLongPressed = false;
            }
        }
        onLongPressed: isLongPressed = true;
        longPressThreshold: 0.3
    }
    DragHandler {
        id: dragHandler
        enabled: kcm.outputModel && kcm.outputModel.rowCount() > 1
        acceptedButtons: Qt.LeftButton
        grabPermissions: PointerHandler.CanTakeOverFromAnything | PointerHandler.TakeOverForbidden
        target: null

        onTranslationChanged: {
            var newX = dragStartPosition.x + translation.x;
            var newY = dragStartPosition.y + translation.y;
            model.position = getAbsolutePosition(Qt.point(newX, newY));
        }
        onActiveChanged: {
            model.interactiveMove = active;
            if (!active) {
                screen.resetTotalSize();
            }
        }
    }
}


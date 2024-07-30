/*
 *   SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.timer 0.1 as TimerPlasmoid

Item {
    id: compactRepresentation

    Layout.minimumHeight: root.inPanel ? Layout.preferredHeight : -1

    Layout.preferredWidth: grid.width
    Layout.preferredHeight: grid.height

    enum LayoutType {
        HorizontalPanel,
        VerticalPanel,
        HorizontalDesktop,
        VerticalDesktop,
        IconOnly
    }

    property int layoutForm

    Binding on layoutForm {
        delayed: true
        value: {
            if (root.inPanel) {
                return root.isVertical ? CompactRepresentation.LayoutType.VerticalPanel : CompactRepresentation.LayoutType.HorizontalPanel;
            }
            if (compactRepresentation.parent.width - iconItem.Layout.preferredWidth >= remainingTimeLabel.contentWidth) {
                return CompactRepresentation.LayoutType.HorizontalDesktop;
            }
            if (compactRepresentation.parent.height - iconItem.Layout.preferredHeight >= remainingTimeLabel.contentHeight) {
                return CompactRepresentation.LayoutType.VerticalDesktop;
            }
            return CompactRepresentation.LayoutType.IconOnly;
        }
    }

    Keys.onUpPressed: adjustSecond(10);
    Keys.onDownPressed: adjustSecond(-10);

    function adjustSecond(value) {
        while (value >= 15 && root.seconds + 1 < 24*60*60) {
            root.seconds += 1;
            value -= 15;
        }
        while (value <= -15 && root.seconds - 1 >= 0) {
            root.seconds -= 1;
            value += 15;
        }
        return value;
    }

    WheelHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        enabled: !root.running
        onWheel: {
            event.accepted = true;
            rotation = compactRepresentation.adjustSecond(rotation);
        }
    }

    GridLayout {
        id: grid

        width: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.HorizontalPanel:
            case CompactRepresentation.LayoutType.HorizontalDesktop:
                return implicitWidth;
            case CompactRepresentation.LayoutType.VerticalPanel:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return compactRepresentation.parent.width;
            case CompactRepresentation.LayoutType.IconOnly:
                return iconItem.Layout.preferredWidth;
            }
        }
        height: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.HorizontalPanel:
            case CompactRepresentation.LayoutType.HorizontalDesktop:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return compactRepresentation.parent.height;
            case CompactRepresentation.LayoutType.VerticalPanel:
                return implicitHeight;
            case CompactRepresentation.LayoutType.IconOnly:
                return iconItem.Layout.preferredHeight;
            }
        }

        rowSpacing: 0
        columnSpacing: rowSpacing
        flow: {
            switch (compactRepresentation.layoutForm) {
            case CompactRepresentation.LayoutType.VerticalPanel:
            case CompactRepresentation.LayoutType.VerticalDesktop:
                return GridLayout.TopToBottom;
            default:
                return GridLayout.LeftToRight;
            }
        }

        Item {
            id: spacerItem
            Layout.fillHeight: true
            visible: layoutForm === CompactRepresentation.LayoutType.VerticalDesktop
        }

        PlasmaComponents3.ToolButton {
            id: iconItem

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: Math.min(compactRepresentation.parent.width, compactRepresentation.parent.height)
            Layout.preferredHeight: Layout.preferredWidth
            visible: root.showTimerToggle

            display: PlasmaComponents3.AbstractButton.IconOnly
            icon.name: {
                if (root.running) {
                    return "chronometer-pause";
                }
                return root.seconds > 0 ? "chronometer-start" : "chronometer";
            }
            text: root.running ? i18nc("@action:button", "Pause Timer") : i18nc("@action:button", "Start Timer")

            onClicked: {
                if (root.seconds === 0) {
                    root.expanded = !root.expanded;
                } else {
                    root.toggleTimer();
                }
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: layoutForm === CompactRepresentation.LayoutType.VerticalPanel || layoutForm === CompactRepresentation.LayoutType.VerticalDesktop
            Layout.maximumWidth: {
                switch (layoutForm) {
                case CompactRepresentation.LayoutType.HorizontalPanel:
                    return Kirigami.Units.gridUnit * 10;
                case CompactRepresentation.LayoutType.HorizontalDesktop:
                    return compactRepresentation.parent.width - iconItem.Layout.preferredWidth;
                default:
                    return -1;
                }
            }
            Layout.maximumHeight: textMetrics.height * 2
            visible: compactRepresentation.layoutForm !== CompactRepresentation.LayoutType.IconOnly ? 1 : 0

            spacing: parent.columnSpacing

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: root.expanded = !root.expanded
            }

            PlasmaComponents3.Label {
                id: titleLabel

                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: root.showTitle && root.title !== ""

                elide: Text.ElideRight
                font.bold: remainingTimeLabel.font.bold
                fontSizeMode: remainingTimeLabel.fontSizeMode
                horizontalAlignment: remainingTimeLabel.horizontalAlignment
                minimumPointSize: remainingTimeLabel.minimumPointSize
                text: root.title
                textFormat: Text.PlainText
            }

            PlasmaComponents3.Label {
                id: remainingTimeLabel

                Layout.fillWidth: parent.Layout.fillWidth
                Layout.fillHeight: true
                Layout.maximumWidth: Layout.fillWidth ? -1 : textMetrics.width
                Layout.minimumWidth: Layout.maximumWidth
                visible: root.showRemainingTime

                TextMetrics {
                    id: textMetrics
                    text: {
                        if (root.isVertical) {
                            return i18ncp("remaining time", "%1s", "%1s", root.seconds);
                        }
                        // make it not jump around: reserve space for one extra digit than reasonable
                        return root.showSeconds ? "44:44:444" : "44:444";
                    }
                    font: remainingTimeLabel.font
                }

                activeFocusOnTab: true
                elide: root.inPanel ? Text.ElideRight : Text.ElideNone
                font.bold: root.alertMode
                fontSizeMode: layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? Text.VerticalFit : Text.HorizontalFit
                horizontalAlignment: layoutForm === CompactRepresentation.LayoutType.HorizontalPanel || layoutForm === CompactRepresentation.LayoutType.HorizontalDesktop ? Text.AlignJustify : Text.AlignHCenter
                minimumPointSize: Kirigami.Theme.smallFont.pointSize
                verticalAlignment: Text.AlignVCenter

                text: {
                    if (root.isVertical) {
                        return i18ncp("remaining time", "%1s", "%1s", root.seconds);
                    }

                    return root.showSeconds ? TimerPlasmoid.Timer.secondsToString(root.seconds, "hh:mm:ss") : TimerPlasmoid.Timer.secondsToString(root.seconds, "hh:mm");
                }
                textFormat: Text.PlainText

                Accessible.name: Plasmoid.toolTipMainText
                Accessible.description: Plasmoid.toolTipSubText
                Accessible.role: Accessible.Button
            }

            PlasmaComponents3.ProgressBar {
                id: remainingTimeProgressBar

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: (parent.visibleChildren.length > 1) ? Math.max(titleLabel.width, remainingTimeLabel.width) : -1
                Layout.minimumWidth: Layout.maximumWidth
                visible: root.showProgressBar

                from: plasmoid.configuration.seconds
                to: 0
                value: root.seconds
            }
        }

        Item {
            Layout.fillHeight: true
            visible: spacerItem.visible
        }
    }
}

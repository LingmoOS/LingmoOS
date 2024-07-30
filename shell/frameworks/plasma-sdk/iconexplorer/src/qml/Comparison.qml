/*
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.4
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.8 as Kirigami

Kirigami.OverlaySheet {
    id: root

    property int comparisonSize

    onAboutToShow: {
        comparisonSize = cuttlefish.iconSize;
        comparisonSlider.value = cuttlefish.iconSizes.indexOf(comparisonSize);
    }

    onOpened: {
        comparisonSlider.forceActiveFocus(Qt.PopupFocusReason);
    }

    implicitWidth: Math.min(parent.width, Kirigami.Units.gridUnit * 50)
    margins: Kirigami.Units.gridUnit * 2
    modal: true
    title: i18nc("@title placeholder is an icon name", "Other versions of %1", preview.iconName)

    footer: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Item {
            Layout.fillWidth: true
        }

        QQC2.Slider {
            id: comparisonSlider

            Layout.preferredWidth: Math.round(parent.width * 0.75)

            from: 0
            to: 6
            stepSize: 1
            snapMode: QQC2.Slider.SnapAlways
            onMoved: {
                comparisonTimer.restart()
            }

            Timer {
                id: comparisonTimer
                running: false
                repeat: false
                interval: 200
                onTriggered: comparisonSize = cuttlefish.iconSizes[comparisonSlider.value]
            }
        }

        QQC2.Label {
            id: iconSizeTextLabel

            Layout.preferredWidth: Math.ceil(iconSizeTextMetrics.advanceWidth)
            horizontalAlignment: Qt.AlignRight
            text: cuttlefish.iconSizes[comparisonSlider.value]

            TextMetrics {
                id: iconSizeTextMetrics
                font: iconSizeTextLabel.font
                text: cuttlefish.iconSizes[cuttlefish.iconSizes.length - 1]
            }
        }

        Item {
            Layout.fillWidth:true
        }
    }

    GridView {
        id: comparisonGrid

        cellWidth: Kirigami.Units.iconSizes.enormous
        cellHeight: cellWidth + 2 * Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing
        implicitWidth: {
            const maxWidth = cuttlefish.width - root.leftMargin - root.rightMargin - root.leftPadding - root.rightPadding;
            return maxWidth < count * cellWidth
                ? maxWidth - maxWidth % cellWidth
                : count * cellWidth;
        }
        implicitHeight: {
            const columns = Math.floor(implicitWidth / cellWidth);
            if (columns === 0) {
                return 0;
            }
            const rows = Math.ceil(count / columns);
            return rows * cellHeight;
        }
        pixelAligned: true
        clip: true
        model: visible ? iconModel.inOtherThemes(preview.iconName, comparisonSize) : null
        delegate: ColumnLayout {
            id: iconColumn
            width: comparisonGrid.cellWidth
            height: comparisonGrid.cellHeight
            spacing: Kirigami.Units.largeSpacing
            Item {
                id: wrapper
                Layout.preferredWidth: comparisonGrid.cellWidth
                Layout.preferredHeight: comparisonGrid.cellWidth
                Kirigami.Icon {
                    readonly property bool hasIcon: Boolean(modelData.iconPath)
                    anchors.centerIn: parent
                    width: root.comparisonSize
                    height: root.comparisonSize
                    source: hasIcon ? modelData.iconPath : "paint-none"
                    color: hasIcon ? "" : Kirigami.Theme.disabledTextColor
                    isMask: !hasIcon
                }
            }
            QQC2.Label {
                id: themeName
                text: modelData.themeName
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                wrapMode: Text.Wrap
                maximumLineCount: 2
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}

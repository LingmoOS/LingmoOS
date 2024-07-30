// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.plasmoid
import org.kde.lingmo.private.shell 2.0
import org.kde.private.mobile.homescreen.folio 1.0 as Folio
import org.kde.lingmouiaddons.formcard 1.0 as FormCard
import org.kde.lingmo.components 3.0 as PC3

import '../delegate'
import '../private'

MouseArea {
    id: root
    property Folio.HomeScreen folio

    property var homeScreen

    signal requestClose()
    onClicked: root.requestClose()

    LingmoUI.Theme.inherit: false
    LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.7)
    }

    RowLayout {
        id: header
        spacing: LingmoUI.Units.largeSpacing
        anchors.left: parent.left
        anchors.leftMargin: LingmoUI.Units.gridUnit
        anchors.top: parent.top
        anchors.topMargin: LingmoUI.Units.gridUnit * 3 + root.homeScreen.topMargin

        PC3.ToolButton {
            Layout.alignment: Qt.AlignVCenter
            icon.name: 'go-previous'
            implicitWidth: LingmoUI.Units.gridUnit * 2
            implicitHeight: LingmoUI.Units.gridUnit * 2
            padding: LingmoUI.Units.smallSpacing
            onClicked: root.requestClose()
        }

        PC3.Label {
            id: heading
            color: 'white'
            text: i18n("Widgets")
            font.weight: Font.Bold
            font.pointSize: LingmoUI.Theme.defaultFont.pointSize * 1.5
        }
    }

    GridView {
        id: gridView
        clip: true
        reuseItems: true

        opacity: 0 // we display with the opacity gradient below

        anchors.top: header.bottom
        anchors.topMargin: LingmoUI.Units.gridUnit
        anchors.left: parent.left
        anchors.leftMargin: root.homeScreen.leftMargin
        anchors.right: parent.right
        anchors.rightMargin: root.homeScreen.rightMargin
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.homeScreen.bottomMargin

        model: widgetExplorer.widgetsModel

        readonly property real maxCellWidth: LingmoUI.Units.gridUnit * 20
        readonly property real intendedCellWidth: LingmoUI.Units.gridUnit * 8
        readonly property int columns: Math.min(5, (width - leftMargin - rightMargin) / intendedCellWidth)

        cellWidth: (width - leftMargin - rightMargin) / columns
        cellHeight: cellWidth + LingmoUI.Units.gridUnit * 3

        readonly property real horizontalMargin: Math.round(width * 0.05)
        leftMargin: horizontalMargin
        rightMargin: horizontalMargin

        MouseArea {
            z: -1
            anchors.fill: parent
            onClicked: root.requestClose()
        }

        delegate: MouseArea {
            id: delegate
            width: gridView.cellWidth
            height: gridView.cellHeight

            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true

            property real zoomScale: pressed ? 0.8 : 1
            transform: Scale {
                origin.x: delegate.width / 2;
                origin.y: delegate.height / 2;
                xScale: delegate.zoomScale
                yScale: delegate.zoomScale
            }

            Behavior on zoomScale { NumberAnimation { duration: 80 } }

            readonly property string pluginName: model.pluginName

            onPressAndHold: {
                root.requestClose();
                folio.HomeScreenState.closeSettingsView();

                let mappedCoords = root.homeScreen.prepareStartDelegateDrag(null, delegate, true);
                const widthOffset = folio.HomeScreenState.pageCellWidth / 2;
                const heightOffset = folio.HomeScreenState.pageCellHeight / 2;

                folio.HomeScreenState.startDelegateWidgetListDrag(
                    mappedCoords.x + mouseX - widthOffset,
                    mappedCoords.y + mouseY - heightOffset,
                    widthOffset,
                    heightOffset,
                    pluginName
                );
            }

            Rectangle {
                id: background
                color: Qt.rgba(255, 255, 255, 0.3)
                visible: delegate.containsMouse
                radius: LingmoUI.Units.cornerRadius
                anchors.fill: parent
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.largeSpacing

                Item {
                    id: iconWidget
                    Layout.fillWidth: true
                    Layout.maximumWidth: delegate.width
                    Layout.preferredHeight: LingmoUI.Units.iconSizes.large
                    Layout.preferredWidth: LingmoUI.Units.iconSizes.large
                    Layout.alignment: Qt.AlignBottom

                    LingmoUI.Icon {
                        anchors.centerIn: parent
                        source: model.decoration
                        visible: model.screenshot == ""
                        implicitWidth: LingmoUI.Units.iconSizes.large
                        implicitHeight: LingmoUI.Units.iconSizes.large
                    }
                    Image {
                        anchors.centerIn: parent
                        fillMode: Image.PreserveAspectFit
                        source: model.screenshot
                        width: LingmoUI.Units.iconSizes.large
                        height: LingmoUI.Units.iconSizes.large
                    }
                }

                PC3.Label {
                    id: heading
                    Layout.fillWidth: true
                    Layout.maximumWidth: delegate.width
                    Layout.alignment: Qt.AlignCenter
                    text: model.name
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    horizontalAlignment: Text.AlignHCenter
                    font.weight: Font.Bold
                }

                PC3.Label {
                    Layout.fillWidth: true
                    Layout.maximumWidth: delegate.width
                    Layout.alignment: Qt.AlignTop
                    // otherwise causes binding loop due to the way the Lingmo sets the height
                    height: implicitHeight
                    text: model.description
                    font.pointSize: LingmoUI.Theme.smallFont.pointSize
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    maximumLineCount: heading.lineCount === 1 ? 3 : 2
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }

    // opacity gradient at grid edges
    FlickableOpacityGradient {
        anchors.fill: gridView
        flickable: gridView
    }

    WidgetExplorer {
        id: widgetExplorer
        containment: Plasmoid
    }
}

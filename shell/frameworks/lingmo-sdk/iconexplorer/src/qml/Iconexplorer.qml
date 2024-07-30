/*
    SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui 2 as LingmoUI

LingmoUI.ApplicationWindow {
    id: cuttlefish
    objectName: "cuttlefish"

    signal itemRightClicked()

    minimumWidth: LingmoUI.Units.gridUnit * 22
    minimumHeight: LingmoUI.Units.gridUnit * 22

    property bool widescreen: cuttlefish.width >= LingmoUI.Units.gridUnit * 44

    property int iconSize: LingmoUI.Units.iconSizes.large

    property alias actions: actions.actions

    readonly property list<int> iconSizes: [
        LingmoUI.Units.iconSizes.small / 2,
        LingmoUI.Units.iconSizes.small,
        LingmoUI.Units.iconSizes.smallMedium,
        LingmoUI.Units.iconSizes.medium,
        LingmoUI.Units.iconSizes.large,
        LingmoUI.Units.iconSizes.huge,
        LingmoUI.Units.iconSizes.enormous,
    ]

    function doScreenshot(montageType) {
        ssLoader.type = montageType
        ssLoader.active = true
    }

    Loader {
        // Use a Loader instead of creating a GMB directly,
        // so if the GMB errors, it doesn't affect Cuttlefish's operation
        id: loader
        source: "GlobalMenuBar.qml"
    }
    Actions {
        id: actions
    }

    Menu {
        id: menu
    }
    Settings {
        id: settings

        property alias x: cuttlefish.x
        property alias y: cuttlefish.y
        property alias width: cuttlefish.width
        property alias height: cuttlefish.height
        property var mainSplitView
    }
    Connections {
        target: loader.item
        function onQuit() { cuttlefish.close() }
        function onZoomIn() { tools.slider.value += 1 }
        function onZoomOut() { tools.slider.value -= 1 }
        function onMontage(montageType) {
            cuttlefish.doScreenshot(montageType)
        }
    }

    Shortcut {
        sequences: [StandardKey.ZoomIn]
        onActivated: tools.slider.value += 1
    }
    Shortcut {
        sequences: [StandardKey.ZoomOut]
        onActivated: tools.slider.value -= 1
    }

    header: Tools {
        id: tools
    }

    QQC2.SplitView {
        id: mainSplitView

        anchors.fill: parent

        QQC2.Pane {
            QQC2.SplitView.fillWidth: true
            QQC2.SplitView.minimumWidth: LingmoUI.Units.gridUnit * 15

            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            padding: 0

            QQC2.ScrollView {
                id: grid

                anchors.fill: parent

                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

                IconGrid {
                    id: iconGrid
                }
            }
        }

        Preview {
            id: previewPane

            QQC2.SplitView.preferredWidth: LingmoUI.Units.gridUnit * 22
            QQC2.SplitView.minimumWidth: LingmoUI.Units.gridUnit * 15

            visible: cuttlefish.widescreen
        }
    }

    footer: ToolsResponsive {
        id: toolsResponsive
        visible: !cuttlefish.widescreen
    }

    Loader {
        active: !cuttlefish.widescreen
        source: "ResponsivePreview.qml"
    }
    Comparison {
        id: comparison
    }
    Connections {
        target: tools
        function onColorschemeChanged(index) {
            toolsResponsive.currentIndex = index
        }
        function onSliderValueChanged(val) {
            toolsResponsive.value = val
        }
    }
    Connections {
        target: toolsResponsive
        function onColorschemeChanged(index) {
            tools.currentIndex = index
        }
        function onSliderValueChanged(val) {
            tools.value = val
        }
    }
    Item {
        id: preview
        property string iconName: "lingmo"
        property string fullPath: ""
        property string category: ""
        property string fileName: ""
        property string type: ""
        property string iconTheme: ""
        property variant sizes: []
        property bool scalable: true
    }
    Loader {
        id: ssLoader
        sourceComponent: Screenshot {}
        active: false

        property int type: Screenshot.MontageType.Dual
        onLoaded: {
            ssLoader.item.shot(type)
        }
        Connections {
            target: ssLoader.item
            function onFinished() {
                ssLoader.active = false
            }
        }
    }

    Component.onCompleted: {
        mainSplitView.restoreState(settings.mainSplitView);
    }

    Component.onDestruction: {
        settings.mainSplitView = mainSplitView.saveState();
    }
}

// SPDX-FileCopyrightText: 2022 Devin Lin <espidev@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import QtQuick.Effects

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.kquickcontrolsaddons 2.0

import org.kde.lingmo.private.containmentlayoutmanager 1.0 as ContainmentLayoutManager
import org.kde.lingmo.private.mobileshell as MobileShell
import org.kde.lingmo.private.mobileshell.shellsettingsplugin as ShellSettings
import org.kde.lingmo.private.mobileshell.state as MobileShellState
import org.kde.lingmo.plasmoid

import org.kde.lingmoui 2.19 as LingmoUI

Item {
    id: delegate

    property int visualIndex: 0
    property real dragFolderAnimationProgress: 0

    property list<LingmoUI.Action> menuActions

    // whether this delegate is a folder
    property bool isFolder

    // folder object
    property var folder
    readonly property string folderName: folder ? folder.name : ""

    // app object
    property var application
    readonly property string applicationName: application ? application.name : ""
    readonly property string applicationStorageId: application ? application.storageId : ""
    readonly property string applicationIcon: application ? application.icon : ""

    signal folderOpenRequested()

    property alias drag: mouseArea.drag
    Drag.active: delegate.drag.active
    Drag.source: delegate
    Drag.hotSpot.x: delegate.width / 2
    Drag.hotSpot.y: delegate.height / 2

    // close context menu if drag move
    onXChanged: {
        if (dialogLoader.item) {
            dialogLoader.item.close()
        }
    }
    onYChanged: {
        if (dialogLoader.item) {
            dialogLoader.item.close()
        }
    }

    function openContextMenu() {
        dialogLoader.active = true;
        dialogLoader.item.open();
    }

    function launch() {
        if (isFolder) {
            folderOpenRequested();
        } else {
            if (application.running) {
                launchAppWithAnim(0, 0, "", applicationName, applicationStorageId);
            } else {
                launchAppWithAnim(delegate.x + (LingmoUI.Units.smallSpacing * 2), delegate.y + (LingmoUI.Units.smallSpacing * 2), delegate.applicationIcon, applicationName, applicationStorageId);
            }
        }
    }

    function launchAppWithAnim(x: int, y: int, source, title: string, storageId: string) {
         if (source !== "") {
            MobileShellState.ShellDBusClient.openAppLaunchAnimationWithPosition(
                Plasmoid.screen,
                source,
                title,
                storageId,
                iconLoader.LingmoUI.ScenePosition.x + iconLoader.width/2,
                iconLoader.LingmoUI.ScenePosition.y + iconLoader.height/2,
                Math.min(iconLoader.width, iconLoader.height));
        }

        application.setMinimizedDelegate(delegate);
        MobileShell.AppLaunch.launchOrActivateApp(application.storageId);
    }

    Loader {
        id: dialogLoader
        active: false

        sourceComponent: LingmoComponents.Menu {
            id: menu
            title: label.text
            closePolicy: LingmoComponents.Menu.CloseOnReleaseOutside | LingmoComponents.Menu.CloseOnEscape

            Repeater {
                model: menuActions
                delegate: LingmoComponents.MenuItem {
                    icon.name: modelData.iconName
                    text: modelData.text
                    onClicked: modelData.triggered()
                }
            }

            onClosed: dialogLoader.active = false
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        property bool inDrag: false

        cursorShape: Qt.PointingHandCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onReleased: {
            delegate.Drag.drop();
            inDrag = false;
        }
        onPressAndHold: { inDrag = true; openContextMenu() }
        drag.target: inDrag ? delegate : undefined

        // grow/shrink animation
        property real zoomScale: 1
        transform: Scale {
            origin.x: mouseArea.width / 2;
            origin.y: mouseArea.height / 2;
            xScale: mouseArea.zoomScale
            yScale: mouseArea.zoomScale
        }

        property bool launchAppRequested: false

        NumberAnimation on zoomScale {
            id: shrinkAnim
            running: false
            duration: ShellSettings.Settings.animationsEnabled ? 80 : 1
            to: ShellSettings.Settings.animationsEnabled ? 0.95 : 1
            onFinished: {
                if (!mouseArea.pressed) {
                    growAnim.restart();
                }
            }
        }

        NumberAnimation on zoomScale {
            id: growAnim
            running: false
            duration: ShellSettings.Settings.animationsEnabled ? 80 : 1
            to: 1
            onFinished: {
                if (mouseArea.launchAppRequested) {
                    delegate.launch();
                    mouseArea.launchAppRequested = false;
                }
            }
        }

        onPressedChanged: {
            if (pressed) {
                growAnim.stop();
                shrinkAnim.restart();
            } else if (!pressed && !shrinkAnim.running) {
                growAnim.restart();
            }
        }

        // launch app handled by press animation
        onClicked: mouse => {
            if (mouse.button === Qt.RightButton) {
                openContextMenu();
            } else {
                launchAppRequested = true;
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: height / 2
            color: mouseArea.pressed ? Qt.rgba(255, 255, 255, 0.2) : "transparent"
        }

        RowLayout {
            id: rowLayout
            anchors {
                fill: parent
                leftMargin: LingmoUI.Units.smallSpacing * 2
                topMargin: LingmoUI.Units.smallSpacing
                rightMargin: LingmoUI.Units.smallSpacing * 2
                bottomMargin: LingmoUI.Units.smallSpacing
            }
            spacing: 0

            Loader {
                id: iconLoader
                Layout.alignment: Qt.AlignLeft
                Layout.minimumWidth: Layout.minimumHeight
                Layout.preferredWidth: Layout.minimumHeight
                Layout.minimumHeight: parent.height
                Layout.preferredHeight: Layout.minimumHeight

                sourceComponent: delegate.isFolder ? folderIconComponent : appIconComponent
            }

            LingmoComponents.Label {
                id: label
                visible: text.length > 0
                textFormat: Text.MarkdownText

                Layout.fillWidth: true
                Layout.leftMargin: LingmoUI.Units.smallSpacing * 2
                Layout.rightMargin: LingmoUI.Units.gridUnit
                wrapMode: Text.WordWrap
                maximumLineCount: 1
                elide: Text.ElideRight

                text: delegate.isFolder ? delegate.folderName : delegate.applicationName

                font.pointSize: LingmoUI.Theme.defaultFont.pointSize
                font.weight: Font.Bold
                color: "white"

                layer.enabled: true
                layer.effect: MobileShell.TextDropShadow {}
            }

            LingmoUI.Icon {
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: LingmoUI.Units.iconSizes.small
                Layout.preferredHeight: LingmoUI.Units.iconSizes.small

                isMask: true
                color: 'white'
                source: 'arrow-right'
                visible: delegate.isFolder

                layer.enabled: true
                layer.effect: MultiEffect {
                    shadowEnabled: true
                    shadowVerticalOffset: 1
                    blurMax: 8
                    shadowOpacity: 0.7
                }
            }
        }
    }

    Component {
        id: appIconComponent

        Item {
            Rectangle {
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.smallSpacing
                color: Qt.rgba(255, 255, 255, 0.2)
                radius: LingmoUI.Units.cornerRadius
                opacity: delegate.dragFolderAnimationProgress
            }

            LingmoUI.Icon {
                id: icon
                anchors.fill: parent
                source: delegate.isFolder ? 'document-open-folder' : delegate.applicationIcon

                transform: Scale {
                    origin.x: icon.width / 2
                    origin.y: icon.height / 2
                    xScale: 1 - delegate.dragFolderAnimationProgress * 0.5
                    yScale: 1 - delegate.dragFolderAnimationProgress * 0.5
                }

                Rectangle {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        bottom: parent.bottom
                    }
                    visible: application ? application.running : false
                    radius: width
                    width: LingmoUI.Units.smallSpacing
                    height: width
                    color: LingmoUI.Theme.highlightColor
                }

                layer.enabled: true
                layer.effect: MultiEffect {
                    shadowEnabled: true
                    shadowVerticalOffset: 1
                    blurMax: 16
                    shadowOpacity: 0.5
                }
            }
        }
    }

    Component {
        id: folderIconComponent

        Item {
            Rectangle {
                id: rect
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.smallSpacing
                color: Qt.rgba(255, 255, 255, 0.2)
                radius: LingmoUI.Units.cornerRadius

                transform: Scale {
                    origin.x: rect.width / 2
                    origin.y: rect.height / 2
                    xScale: 1 + delegate.dragFolderAnimationProgress * 0.5
                    yScale: 1 + delegate.dragFolderAnimationProgress * 0.5
                }
            }

            Grid {
                id: grid
                anchors.fill: parent
                anchors.margins: LingmoUI.Units.smallSpacing * 2
                columns: 2
                spacing: LingmoUI.Units.smallSpacing

                property var previews: model.folder.appPreviews

                Repeater {
                    model: grid.previews
                    delegate: LingmoUI.Icon {
                        implicitWidth: (grid.width - LingmoUI.Units.smallSpacing) / 2
                        implicitHeight: (grid.width - LingmoUI.Units.smallSpacing) / 2
                        source: modelData.icon

                        layer.enabled: true
                        layer.effect: MultiEffect {
                            shadowEnabled: true
                            shadowVerticalOffset: 1
                            blurMax: 16
                            shadowOpacity: 0.6
                        }
                    }
                }
            }
        }
    }
}




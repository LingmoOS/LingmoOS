/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.3 as Controls
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kirigami 2.20 as Kirigami

//for the "simple mode"
import org.kde.plasma.wallpapers.image 2.0 as Wallpaper
import org.kde.kquickcontrolsaddons 2.0 as Addons

AppletConfiguration {
    id: root
    isContainment: true

    internalDialog.visible: false
    internalDialog.width: root.width < root.height ? root.width : Math.min(root.width, Math.max(internalDialog.implicitWidth, Kirigami.Units.gridUnit * 45))
    internalDialog.height: Math.min(root.height, Math.max(internalDialog.implicitHeight, Kirigami.Units.gridUnit * 29))

    readonly property bool horizontal: root.width > root.height

//BEGIN model
    globalConfigModel: globalContainmentConfigModel

    ConfigModel {
        id: globalContainmentConfigModel
        ConfigCategory {
            name: i18nd("plasma_shell_org.kde.plasma.desktop", "Wallpaper")
            icon: "preferences-desktop-wallpaper"
            source: "ConfigurationContainmentAppearance.qml"
        }
    }
//END model

    Controls.Drawer {
        id: imageWallpaperDrawer
        edge: root.horizontal ? Qt.LeftEdge : Qt.BottomEdge
        visible: true
        dragMargin: 0

        onClosed: {
            if (!root.internalDialog.visible) {
                configDialog.close()
            }
        }
        onOpened: {
            wallpapersView.forceActiveFocus()
        }
        implicitWidth: Kirigami.Units.gridUnit * 10
        implicitHeight: Kirigami.Units.gridUnit * 8
        width: root.horizontal ? implicitWidth : root.width
        height: root.horizontal ? root.height : implicitHeight
        Wallpaper.ImageBackend {
            id: imageWallpaper
        }
        background: null

        ListView {
            id: wallpapersView
            anchors.fill: parent
            orientation: root.horizontal ? ListView.Vertical : ListView.Horizontal
            keyNavigationEnabled: true
            highlightFollowsCurrentItem: true
            snapMode: ListView.SnapToItem
            model: imageWallpaper.wallpaperModel
            onCountChanged: currentIndex =  Math.min(model.indexOf(configDialog.wallpaperConfiguration["Image"]), model.rowCount()-1)
            footer: Controls.Control {
                z: 999
                width: root.horizontal ? parent.width : implicitWidth
                height: root.horizontal ? implicitHeight : parent.height
                leftPadding: Kirigami.Units.gridUnit
                topPadding: Kirigami.Units.gridUnit
                rightPadding: Kirigami.Units.gridUnit
                bottomPadding: Kirigami.Units.gridUnit

                contentItem: ColumnLayout {
                    Controls.Button {
                        icon.name: "configure"
                        text: i18nd("plasma_shell_org.kde.plasma.desktop", "Customize...")
                        onClicked: {
                            print(wallpapersView.currentIndex)
                            internalDialog.visible = true;
                            imageWallpaperDrawer.close()
                        }
                    }
                }
                background: Rectangle {
                    color: Qt.rgba (0, 0, 0, 0.3)
                }
            }
            headerPositioning: ListView.PullBackHeader
            delegate: Controls.ItemDelegate {
                width: root.horizontal ? parent.width : height * (root.Screen.width / root.Screen.height)
                height: root.horizontal ? width / (root.Screen.width / root.Screen.height) : parent.height
                padding: wallpapersView.currentIndex === index ? Kirigami.Units.gridUnit / 4 : Kirigami.Units.gridUnit / 2
                leftPadding: padding
                topPadding: padding
                rightPadding: padding
                bottomPadding: padding
                Behavior on padding {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }

                property bool isCurrent: configDialog.wallpaperConfiguration["Image"] == model.path
                onIsCurrentChanged: {
                    if (isCurrent) {
                        wallpapersView.currentIndex = index;
                    }
                }
                
                z: wallpapersView.currentIndex === index ? 2 : 0
                contentItem: Item {
                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: Kirigami.Units.iconSizes.large
                        height: width
                        source: "view-preview"
                        visible: !walliePreview.visible
                    }

                    Addons.QPixmapItem {
                        id: walliePreview
                        anchors.fill: parent
                        visible: model.screenshot != null
                        smooth: true
                        pixmap: model.screenshot
                        fillMode: Image.PreserveAspectCrop
                        
                    }
                }
                onClicked: {
                    configDialog.currentWallpaper = "org.kde.image";
                    configDialog.wallpaperConfiguration["Image"] = model.path;
                    configDialog.applyWallpaper()
                }
                Keys.onReturnPressed: {
                    clicked();
                }
                background: Item {
                    Rectangle {
                        anchors {
                            fill: parent
                            margins: wallpapersView.currentIndex === index ? 0 : Kirigami.Units.gridUnit / 4
                            Behavior on margins {
                                NumberAnimation {
                                    duration: Kirigami.Units.longDuration
                                    easing.type: Easing.InOutQuad
                                }
                            }
                        }
                        radius: Kirigami.Units.gridUnit / 4
                    }
                }
            }
        }
    }
}

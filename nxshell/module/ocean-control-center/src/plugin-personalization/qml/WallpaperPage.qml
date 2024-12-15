// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D

OceanUIObject {
    OceanUITitleObject {
        name: "wallpaperTitle"
        weight: 10
        parentName: "personalization/wallpaper"
        displayName: qsTr("wallpaper")
    }
    OceanUIObject {
        name: "wallpaperStatusGroup"
        parentName: "personalization/wallpaper"
        weight: 100
        pageType: OceanUIObject.Item
        page: OceanUIRowView { }
    }

    OceanUIObject {
        name: "wallpaparDisplayArea"
        parentName: "personalization/wallpaper/wallpaperStatusGroup"
        weight: 200
        pageType: OceanUIObject.Item
        page: RowLayout {
            Item {
                width: 270
                height: 180

                Image {
                    id: image
                    anchors.fill: parent
                    source: oceanuiData.model.wallpaperMap[oceanuiData.model.currentSelectScreen]
                    sourceSize: Qt.size(image.width, image.height)
                    visible: false
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                }

                OpacityMask {
                    anchors.fill: parent
                    source: image
                    maskSource: Rectangle {
                        implicitWidth: image.width
                        implicitHeight: image.height
                        radius: 6
                    }
                }
            }
        }
    }
    OceanUIObject {
        name: "wallpaperSetGroup"
        parentName: "personalization/wallpaper/wallpaperStatusGroup"
        weight: 300
        pageType: OceanUIObject.Item
        page: OceanUIGroupView { isGroup: false }

        OceanUIObject {
            name: "wallpaperSetItemGroup"
            parentName: "personalization/wallpaper/wallpaperStatusGroup/wallpaperSetGroup"
            displayName: qsTr("Window rounded corners")
            weight: 10
            pageType: OceanUIObject.Item
            page: OceanUIGroupView { }

            OceanUIObject {
                name: "whenTheLidIsClosed"
                parentName: "personalization/wallpaper/wallpaperStatusGroup/wallpaperSetGroup/wallpaperSetItemGroup"
                displayName: qsTr("Select Screen")
                weight: 10
                pageType: OceanUIObject.Editor
                page: D.ComboBox {
                    flat: true
                    model: oceanuiData.model.screens
                    onCurrentTextChanged: {
                        oceanuiData.model.currentSelectScreen = currentText
                    }
                }
            }
            OceanUIObject {
                name: "whenTheLidIsClosed"
                parentName: "personalization/wallpaper/wallpaperStatusGroup/wallpaperSetGroup/wallpaperSetItemGroup"
                displayName: qsTr("wave of the blue")
                visible: false
                weight: 100
                pageType: OceanUIObject.Editor
                page: D.ComboBox {
                    width: 100
                    flat: true
                    model: ["full screen"]
                }
            }
            OceanUIObject {
                name: "whenTheLidIsClosed"
                parentName: "personalization/wallpaper/wallpaperStatusGroup/wallpaperSetGroup/wallpaperSetItemGroup"
                displayName: qsTr("auto change wallpaper")
                visible: false
                weight: 200
                pageType: OceanUIObject.Editor
                page: D.ComboBox {
                    width: 100
                    flat: true
                    model: ["none"]
                }
            }
        }

        OceanUIObject {
            name: "wallpaperSetButtonGroup"
            parentName: "personalization/wallpaper/wallpaperStatusGroup/wallpaperSetGroup"
            weight: 300
            pageType: OceanUIObject.Item
            page: OceanUIGroupView {
                isGroup: false
            }

            OceanUIObject {
                name: "wallpaperSetButton"
                parentName: "personalization/wallpaper/wallpaperStatusGroup/wallpaperSetGroup/wallpaperSetButtonGroup"
                weight: 2
                pageType: OceanUIObject.Item
                page: RowLayout {
                    Item {
                        Layout.fillWidth: true
                    }
                    D.Button {
                        visible: false
                        text: "Add Image"
                    }
                    Item {
                        Layout.fillWidth: true
                    }

                    D.Button {
                        visible: false
                        text: "Add Folder"
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }

        }
    }
    OceanUIObject {
        name: "screenAndSuspendTitle"
        parentName: "personalization/wallpaper"
        // displayName: qsTr("你的图片")
        displayName: qsTr("System Wallapers")
        weight: 300
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: WallpaperSelectView {
            model: oceanuiData.model.wallpaperModel
            currentItem: oceanuiData.model.wallpaperMap[oceanuiData.model.currentSelectScreen]
            onWallpaperSelected: (url, isDark, isLock) => {
                                     if (isLock) {
                                         oceanuiData.worker.setLockBackForMonitor(oceanuiData.model.currentSelectScreen, url, isDark)
                                     } else {
                                         oceanuiData.worker.setBackgroundForMonitor(oceanuiData.model.currentSelectScreen, url, isDark)
                                     }
                                 }
        }
    }

    OceanUIObject {
        name: "screenAndSuspendTitle"
        parentName: "personalization/wallpaper"
        displayName: qsTr("每周推荐")
        weight: 300
        visible: false
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: WallpaperSelectView {
            model: 10
        }
    }

    OceanUIObject {
        name: "screenAndSuspendTitle"
        parentName: "personalization/wallpaper"
        displayName: qsTr("炫彩")
        weight: 300
        visible: false
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: WallpaperSelectView {
            model: 34
        }
    }

    OceanUIObject {
        name: "screenAndSuspendTitle"
        parentName: "personalization/wallpaper"
        displayName: qsTr("颜色")
        visible: false
        weight: 300
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: WallpaperSelectView {
            model: 20
        }
    }

    OceanUITitleObject {
        name: "sreenSaverTitle"
        weight: 400
        visible: false
        parentName: "personalization/wallpaper"
        displayName: qsTr("屏保")
    }
    OceanUIObject {
        name: "sreenSaverStatusGroup"
        parentName: "personalization/wallpaper"
        weight: 500
        visible: false
        pageType: OceanUIObject.Item
        page: OceanUIRowView { }
    }

    OceanUIObject {
        name: "sreenSaverDisplayArea"
        parentName: "personalization/wallpaper/sreenSaverStatusGroup"
        weight: 600
        visible: false
        pageType: OceanUIObject.Item
        page: RowLayout {
            Item {
                width: 270
                height: 180

                Image {
                    anchors.fill: parent
                    id: screenSaverimage
                    source: "file:///usr/share/wallpapers/lingmo/cristina-gottardi-wndpWTiDuT0-unsplash.jpg"
                    visible: false
                    fillMode: Image.PreserveAspectCrop
                }

                OpacityMask {
                    anchors.fill: parent
                    source: screenSaverimage
                    maskSource: Rectangle {
                        implicitWidth: screenSaverimage.width
                        implicitHeight: screenSaverimage.height
                        radius: 6
                    }
                }
            }
        }
    }
    OceanUIObject {
        name: "sreenSaverSetGroup"
        parentName: "personalization/wallpaper/sreenSaverStatusGroup"
        weight: 700
        visible: false
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {  }

        OceanUIObject {
            name: "whenTheLidIsClosed"
            parentName: "personalization/wallpaper/sreenSaverStatusGroup/sreenSaverSetGroup"
            displayName: qsTr("照片")
            weight: 10
            pageType: OceanUIObject.Editor
            page: D.Button {
                text: "设置"
                onClicked: {
                    screenSaverDialog.show()
                }

                D.DialogWindow {
                    id: screenSaverDialog
                    minimumWidth: 400
                    minimumHeight: 220
                    title: "照片设置"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.topMargin: 10
                        GridLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            rows:3
                            columns: 2
                            Label {
                                text: "来源:"
                            }

                            D.TextField {
                                Layout.fillWidth: true
                            }
                            Label {
                                text: "来源:"
                            }

                            D.TextField {
                                Layout.fillWidth: true
                            }
                            Item {}
                            D.CheckBox {
                                text: "随机排列图片的顺序"
                            }
                        }

                        D.DialogButtonBox {
                            Layout.fillWidth: true
                            alignment: Qt.AlignCenter
                            standardButtons: DialogButtonBox.NoButton
                            Button {
                                text: qsTr("Cancel")
                                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                            }
                            Button {
                                text: qsTr("Save")
                                highlighted: true
                                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                            }
                        }
                    }
                }
            }
        }
        OceanUIObject {
            name: "whenTheLidIsClosed"
            parentName: "personalization/wallpaper/sreenSaverStatusGroup/sreenSaverSetGroup"
            displayName: qsTr("闲置时间")
            weight: 100
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                width: 100
                flat: true
                model: ["闲置时间"]
            }
        }
        OceanUIObject {
            name: "whenTheLidIsClosed"
            parentName: "personalization/wallpaper/sreenSaverStatusGroup/sreenSaverSetGroup"
            displayName: qsTr("auto change wallpaper")
            weight: 200
            pageType: OceanUIObject.Editor
            page: D.ComboBox {
                width: 100
                flat: true
                model: ["30分钟"]
            }
        }
    }
    OceanUIObject {
        name: "screenAndSuspendTitle"
        parentName: "personalization/wallpaper"
        displayName: qsTr("炫彩")
        visible: false
        weight: 800
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        page: WallpaperSelectView {
            model: 9
        }
    }
}

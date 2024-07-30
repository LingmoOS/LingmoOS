// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import org.kde.lingmoui 2.19 as LingmoUI
import QtQuick.Controls 2.15 as Controls
import Qt.labs.folderlistmodel 2.4
import QtMultimedia

Controls.Popup {
    id: menuPopupArea
    width: parent.width / 2
    height:  parent.height / 2
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    dim: true

    onOpened: {
        if(testModelView.count > 0){
            testModelView.forceActiveFocus()
        } else {
            fileModelFolderSelector.forceActiveFocus()
        }
    }

    Controls.Overlay.modeless: Rectangle {
        color: Qt.rgba(LingmoUI.Theme.backgroundColor.r, LingmoUI.Theme.backgroundColor.g, LingmoUI.Theme.backgroundColor.b, 0.77)
    }

    background: Rectangle {
        LingmoUI.Theme.colorSet: LingmoUI.Theme.View
        color: LingmoUI.Theme.backgroundColor
        border.color: "black"
    }

    FolderListModel {
        id: folderModel
        folder: Qt.resolvedUrl("file://" + HomeDirectory)
        rootFolder: Qt.resolvedUrl("file:///")
        showDirs: true
        showFiles: false
        showDotAndDotDot: true
        nameFilters: [".*"]
    }

    FolderListModel {
        id: fileModel
        folder: folderModel.folder
        showDirs: false
        showDotAndDotDot: false
        showOnlyReadable: true
        nameFilters: ["*.WEBM", "*.MPG", "*.MP2", "*.MPEG", "*.MPE", "*.MPV", "*.OGG", "*.MP4", "*.M4P", "*.M4V", "*.MKV", "*.AVI", "*.WMV", "*.MOV", "*.QT", "*.FLV", "*.SWF", "*.AVCHD", "*.264", "*.3G2", "*.3GP", "*.3MM", "*.3P2", "*.DIVX", "*.DVM", "*.DVR", "*.DVX", "*.F4V", "*.M2V", "*.M4E", "*.M4U", "*.M4V", "*.MJP", "*.MJ2", "*.MK3D", "*.MP4V", "*.MPG2", "*.MQV", "*.OGV", "*.TS", "*.VP6", "*.VP7", "*.VP8", "*.VP9", "*.VRO", "*.XVID", "*.webm", "*.mpg", "*.mp2", "*.mpeg", "*.mpe", "*.mpv", "*.ogg", "*.mp4", "*.m4p", "*.m4v", "*.mkv", "*.avi", "*.wmv", "*.mov", "*.qt", "*.flv", "*.swf", "*.avchd", "*.264", "*.3g2", "*.3gp", "*.3mm", "*.3p2", "*.divx", "*.dvm", "*.dvr", "*.dvx", "*.f4v", "*.m2v", "*.m4e", "*.m4u", "*.m4v", "*.mjp", "*.mj2", "*.mk3d", "*.mp4v", "*.mpg2", "*.mqv", "*.ogv", "*.ts", "*.vp6", "*.vp7", "*.vp8", "*.vp9", "*.vro", "*.xvid"]
    }

    Item {
        width: parent.width
        height: parent.height

        RowLayout {
            id: headerAreaBMLPg1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: LingmoUI.Units.gridUnit * 2

            Rectangle {
                color: LingmoUI.Theme.backgroundColor
                radius: 10
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: LingmoUI.Units.smallSpacing

                Controls.Label {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing
                    anchors.topMargin: LingmoUI.Units.smallSpacing
                    anchors.bottomMargin: LingmoUI.Units.smallSpacing
                    anchors.rightMargin: LingmoUI.Units.largeSpacing
                    color: LingmoUI.Theme.textColor
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    text: folderModel.folder.toString().replace("file://", "")
                }
            }

            Controls.Label {
                id: backbtnlabelHeading
                text: i18n("Press 'esc' or the [←] Back button to close")
                Layout.alignment: Qt.AlignRight
            }
        }

        LingmoUI.Separator {
            id: headrSeptBml
            anchors.top: headerAreaBMLPg1.bottom
            width: parent.width
            height: 1
        }

        RowLayout {
            anchors.top: headrSeptBml.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: LingmoUI.Units.largeSpacing

            ListView {
                id: fileModelFolderSelector
                model: folderModel
                Layout.preferredWidth: parent.width * 0.30
                Layout.fillHeight: true
                keyNavigationEnabled: true
                highlightFollowsCurrentItem: true
                highlightRangeMode: GridView.ApplyRange
                spacing: LingmoUI.Units.smallSpacing
                clip: true
                KeyNavigation.right: testModelView
                delegate: MenuButton {
                    iconSource: "folder"
                    text: i18n(fileName)

                    onClicked: (mouse)=> {
                        folderModel.folder = fileUrl
                    }
                }
            }

            GridView {
                id: testModelView
                model: fileModel
                Layout.fillWidth: true
                Layout.fillHeight: true
                cellWidth: parent.width / 6
                cellHeight: cellWidth
                keyNavigationEnabled: true
                highlightFollowsCurrentItem: true
                highlightRangeMode: GridView.ApplyRange
                snapMode: GridView.SnapToRow
                cacheBuffer: width
                highlightMoveDuration: LingmoUI.Units.longDuration
                clip: true
                KeyNavigation.left: fileModelFolderSelector
                enabled: fileModel.count > 0 ? 1 :0
                delegate: FileItemDelegate {
                    onClicked: (mouse)=> {
                        window.videoSource = fileUrl
                        menuPopupArea.close()
                    }
                }

                onCountChanged: {
                    if(testModelView.count > 0){
                        testModelView.forceActiveFocus()
                        testModelView.currentIndex = 0
                        fileModelFolderSelector.currentIndex = 0
                    } else {
                        fileModelFolderSelector.forceActiveFocus()
                        fileModelFolderSelector.currentIndex = 0
                    }
                }

                move: Transition {
                    SmoothedAnimation {
                        property: "x"
                        duration: LingmoUI.Units.longDuration
                    }
                }
            }
        }
    }
}

/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts

import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {

    signal configurationChanged

    property var cfg_pathList: []

    function addPath(object) {
        pathModel.append( object )
        cfg_pathList.push( JSON.stringify(object) )
        configurationChanged();
    }

    function removePath(index) {
        if(pathModel.count > 0) {
            pathModel.remove(index)
            cfg_pathList.splice(index,1)
            configurationChanged();
        }
    }

    Component.onCompleted: {
        // Load the list back in
        var list = plasmoid.configuration.pathList
        cfg_pathList = []
        for(var i in list) {
            addPath( JSON.parse(list[i]) )
        }
    }

    view: ListView {
        id: pathsList

        model: ListModel {
            id: pathModel
        }

        delegate: Kirigami.SwipeListItem {
            id: folderDelegate

            width: pathsList.width

            contentItem: QQC2.Label {
                Layout.fillWidth: true
                text: String(model.path).replace("file://", "")
                textFormat: Text.PlainText
            }

            actions: [
                Kirigami.Action {
                    icon.name: "list-remove"
                    tooltip: i18nd("plasma_wallpaper_org.kde.image", "Remove path")
                    onTriggered: removePath(model.index)
                }
            ]
        }

        FileDialog {
            id: fileDialog

            title: i18nc("@title:window", "Choose Files")
            currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
            fileMode: FileDialog.OpenFiles

            // TODO get valid filter list from native code?
            //nameFilters: [ "Image files (*.png *.jpg)", "All files (*)" ]
            //selectedNameFilter: "All files (*)"

            onAccepted: {
                console.log("Accepted: " + selectedFiles)

                for (var i = 0; i < selectedFiles.length; ++i) {
                    var item = { 'path':selectedFiles[i], 'type':'file' }
                    addPath(item)
                }
            }

            onRejected: {
                console.log("Canceled")
            }
        }

        FolderDialog {
            id: folderDialog

            visible: false
            title: i18nc("@title:window", "Choose a Folder")
            currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]

            onAccepted: {
                console.log("Accepted: " + selectedFolder)

                var item = { 'path':selectedFolder, 'type':'folder' }
                addPath(item)
            }

            onRejected: {
                console.log("Canceled")
            }
        }
    }

    footer: RowLayout {
        QQC2.Button {
            icon.name: "folder-new"
            onClicked: folderDialog.visible = true
            text: i18nc("@action:button", "Add Folder…")
        }

        QQC2.Button {
            icon.name: "document-new"
            onClicked: fileDialog.visible = true
            text: i18nc("@action:button", "Add Files…")
        }
        Item {
            Layout.fillWidth: true
        }
    }
}

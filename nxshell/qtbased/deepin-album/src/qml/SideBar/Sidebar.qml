// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import org.deepin.dtk 1.0

import org.deepin.album 1.0 as Album

import "../Control"
import "../PreviewImageViewer/Utils"
import "../"
ScrollView {
    id: sidebarScrollView
    clip: true
    signal sideBarListChanged(string type, string displayName)
    property int currentImportCustomIndex: 0 //自动导入相册当前索引值
    property int currentCustomIndex: 0 //自定义相册当前索引值
    property var devicePaths : albumControl.getDevicePaths()
    property var albumPaths : albumControl.getAlbumPaths(GStatus.currentCustomAlbumUId)
    property var importAlbumNames : {
        GStatus.albumImportChangeList
        albumControl.getImportAlubumAllNames()
    }
    property var customAlbumNames : {
        GStatus.albumChangeList
        albumControl.getAllCustomAlbumName()
    }
    property ListModel deviceListModel: ListModel {}
    property ListModel importListModel: ListModel {}
    property ListModel customListModel: ListModel {}

    onXChanged: {
        GStatus.sideBarX = x;
    }

    // 设备路径有变更，刷新设备列表
    onDevicePathsChanged: {
        deviceListModel.clear()
        for (var i = 0; i < devicePaths.length; i++) {
            var tempDevice = {}
            tempDevice.checked = false
            tempDevice.icon = "iphone"
            tempDevice.displayName = albumControl.getDeviceNames()[i]
            tempDevice.uuid = String(i)
            tempDevice.editable = false
            tempDevice.deleteable = false
            tempDevice.path = devicePaths[i]
            deviceListModel.append(tempDevice)
        }
    }

    // 导入相册列表有变更，刷新导入相册侧边栏
    onImportAlbumNamesChanged: {
        importListModel.clear()
        for (var i = 0; i < importAlbumNames.length; i++) {
            var tempImort = {}
            tempImort.checked = false
            tempImort.icon = "custom"
            tempImort.displayName = importAlbumNames[i]
            tempImort.uuid = albumControl.getImportAlubumAllId()[i]
            tempImort.editable = false
            tempImort.deleteable = true
            importListModel.append(tempImort)
        }
    }

    // 导入相册列表有变更，刷新导入相册侧边栏
    onCustomAlbumNamesChanged: {
        customListModel.clear()
        for (var i = 0; i < customAlbumNames.length; i++) {
            var tempCustom= {}
            tempCustom.checked = false
            tempCustom.icon = "item"
            tempCustom.displayName = customAlbumNames[i]
            tempCustom.uuid = albumControl.getAllCustomAlbumId()[i]
            tempCustom.editable = true
            tempCustom.deleteable = true
            customListModel.append(tempCustom)
        }
    }

    // 返回到合集
    function backCollection() {
        gallerySideBar.view.currentIndex = 0
        gallerySideBar.view.currentItem.checked = true
        GStatus.currentViewType = Album.Types.ViewCollecttion
        GStatus.searchEditText = ""
        gallerySideBar.view.currentItem.forceActiveFocus()
    }

    // 返回到系统相册（用于删除相册后，当前导航栏定位到离删除相册最近的上一个相册）
    function backSystemAlbum() {
        if (sysListModel.count > 0) {
            systemSideBar.view.currentIndex = sysListModel.count - 1
            systemSideBar.view.currentItem.checked = true
            GStatus.currentViewType = Album.Types.ViewCustomAlbum
            GStatus.currentCustomAlbumUId = sysListModel.get(systemSideBar.view.currentIndex).uuid
            GStatus.searchEditText = ""
            systemSideBar.view.currentItem.forceActiveFocus()
        } else if (sysListModel.count === 0) {
            backCollection()
        }
    }

    // 删除自动导入相册
    function deleteImportAlbum() {
        var delAlbumName = albumControl.getImportAlubumAllNames()[currentImportCustomIndex]
        albumControl.removeAlbum(GStatus.currentCustomAlbumUId)
        albumControl.removeCustomAutoImportPath(GStatus.currentCustomAlbumUId)
        GStatus.albumImportChangeList = !GStatus.albumImportChangeList
        if(currentImportCustomIndex >= albumControl.getImportAlubumAllId().length){
            currentImportCustomIndex = albumControl.getImportAlubumAllId().length - 1
        }
        if (currentImportCustomIndex >= 0 && currentImportCustomIndex < albumControl.getImportAlubumAllId().length){
            importSideBar.view.currentIndex = currentImportCustomIndex
            importSideBar.view.currentItem.checked = true
            GStatus.currentViewType = Album.Types.ViewCustomAlbum
            GStatus.currentCustomAlbumUId = albumControl.getImportAlubumAllId()[currentImportCustomIndex]
            importSideBar.view.currentItem.forceActiveFocus()
        }

        DTK.sendMessage(thumbnailImage, qsTr("Album “%1” removed").arg(qsTr(delAlbumName)), "notify_checked")

        // 自动导入相册被清空，返回到上一级相册
        if(albumControl.getImportAlubumCount() === 0){
            backSystemAlbum()
        }
    }

    // 删除自定义相册
    function deleteCustomAlbum() {
        var delAlbumName = albumControl.getCustomAlbumByUid(GStatus.currentCustomAlbumUId)
        albumControl.removeAlbum(GStatus.currentCustomAlbumUId)
        GStatus.albumChangeList = !GStatus.albumChangeList
        if(currentCustomIndex >= albumControl.getAllCustomAlbumId().length ){
            currentCustomIndex = albumControl.getAllCustomAlbumId().length - 1
        }
        if (currentCustomIndex >= 0 && currentCustomIndex < albumControl.getAllCustomAlbumId().length){
            customSideBar.view.currentIndex = currentCustomIndex
            customSideBar.view.currentItem.checked = true
            GStatus.currentViewType = Album.Types.ViewCustomAlbum
            GStatus.currentCustomAlbumUId = albumControl.getAllCustomAlbumId()[currentCustomIndex]
            customSideBar.view.currentItem.forceActiveFocus()
        }

        DTK.sendMessage(thumbnailImage, qsTr("Album “%1” removed").arg(qsTr(delAlbumName)), "notify_checked")

        // 自定义相册被清空，返回到上一级相册
        if(albumControl.getAllCustomAlbumId().length === 0){
            backSystemAlbum()
        }
    }

    //侧边栏如果是关闭状态，侧边栏会自动打开
    function autoShowSliderAnimation() {
        if (leftSidebar.x !== 0) {
            showSliderAnimation.start()
        }
    }

    Column {
        id: column
        spacing: 0
        ButtonGroup {id: paneListGroup}

        // 照片库侧边栏
        SideBarItem {
            id: gallerySideBar
            Layout.alignment:  Qt.AlignTop; Layout.topMargin: 15
            title: qsTr("Gallery")
            group: paneListGroup
            sideModel: ListModel {
                ListElement{checked: true; icon: "images"; displayName: qsTr("Collection"); uuid: "collection"; editable: false; deleteable: false}
                ListElement{checked: false; icon: "import_left"; displayName: qsTr("Import"); uuid: "import"; editable: false; deleteable: false}
                ListElement{checked: false; icon: "collection"; displayName: qsTr("Favorites"); uuid: "favorites"; editable: false; deleteable: false}
                ListElement{checked: false; icon: "trash"; displayName: qsTr("Trash"); uuid: "trash"; editable: false; deleteable: false}
            }

            onItemClicked: (uuid)=> {
                GStatus.currentViewType = view.currentIndex + 2
                // 导航页选中我的收藏时，设定自定相册索引为0，使用CutomAlbum控件按自定义相册界面逻辑显示我的收藏内容
                if (GStatus.currentViewType === Album.Types.ViewFavorite) {
                    GStatus.currentCustomAlbumUId = 0
                } else {
                    GStatus.currentCustomAlbumUId = -1
                }

                GStatus.searchEditText = ""
            }
        }

        Item {width: parent.width; height: 10}

        // 设备列表侧边栏
        SideBarItem {
            id: deviceSideBar
            visible: devicePaths.length
            Layout.alignment:  Qt.AlignTop; Layout.topMargin: 15
            title: qsTr("Device")
            showRemoveDeviceBtn: true
            group: paneListGroup
            sideModel: deviceListModel

            Connections {
                target: deviceSideBar
                function onRemoveDeviceBtnClicked(uuid) {
                    var index = deviceSideBar.indexFromUuid(uuid)
                    if (index !== -1) {
                        if (sidebarScrollView.devicePaths.length === 1){
                            sidebarScrollView.backCollection()
                        } else {
                            if (index - 1 >= 0) {
                                GStatus.currentDevicePath = sidebarScrollView.devicePaths[index - 1]
                            } else {

                            }

                            forceActiveFocus()
                        }
                        albumControl.unMountDevice(sidebarScrollView.devicePaths[index])
                    }                
                }
            }

            Connections {
                target: albumControl
                function onSigAddDevice(path) {
                    for (var i = 0; i < deviceListModel.length; i++) {
                        if (deviceListModel[i].path === path) {
                            deviceSideBar.view.currentIndex = i
                            deviceSideBar.view.currentItem.checked = true
                            break
                        }
                    }

                    autoShowSliderAnimation()
                }
            }

            onItemCheckedChanged: (index, checked)=> {
                if (checked) {
                    GStatus.currentDevicePath = deviceListModel.get(index).path
                    GStatus.currentViewType = Album.Types.ViewDevice
                    GStatus.searchEditText = ""
                    forceActiveFocus()
                }

                autoShowSliderAnimation()
            }
        }

        Item {width: parent.width; height: 10; visible: devicePaths.length}

        // 相册侧边栏(系统相册 + 自定导入相册 + 自定义相册)
        SideBarItem {
            id: systemSideBar
            title: qsTr("Albums")
            group: paneListGroup

            ListModel {
                id: sysListModel
            }

            sideModel: sysListModel

            // "添加相册"按钮
            action: FloatingButton {
                width: 21; height: 21
                checked: false
                palette: systemSideBar.palette
                icon {
                    name: "list-add"
                    width: 21
                    height: 21
                }

                ToolTip.visible: hovered
                ToolTip.text: qsTr("Add an album")
                onClicked: {
                    newAlbum.setNormalEdit()
                    newAlbum.isChangeView = true
                    newAlbum.show()
                    forceActiveFocus()
                }
            }

            onItemClicked: (uuid)=> {
                GStatus.currentViewType = Album.Types.ViewCustomAlbum
                GStatus.currentCustomAlbumUId = uuid
                GStatus.searchEditText = ""
                forceActiveFocus()
            }

            onItemRightClicked: {
                if (sidebarScrollView.albumPaths.length > 0) {
                    systemMenu.popup()
                }
            }

            Component.onCompleted: {
                //根据文件夹情况刷新当前的默认路径相册显示
                //1: 截图，2: 相机，3: 画板
                if(albumControl.isDefaultPathExists(1)) {
                    sysListModel.append({checked: false, icon: "screenshot", displayName: qsTr("Screen Capture"), uuid: "1", editable: false})
                }

                if(albumControl.isDefaultPathExists(2)) {
                    sysListModel.append({checked: false, icon: "camera", displayName: qsTr("Camera"), uuid: "2", editable: false})
                }

                if(albumControl.isDefaultPathExists(3)) {
                    sysListModel.append({checked: false, icon: "draw", displayName: qsTr("Draw"), uuid: "3", editable: false})
                }
            }
        }

        // 自动导入的相册列表(拖拽图片文件夹导入的相册列表)
        SideBarItem {
            id: importSideBar
            showTitle: false
            group: paneListGroup
            sideModel: importListModel

            Connections {
                target: albumControl
                function onSigAddCustomAlbum(UID) {
                    for (var i = 0; i < importListModel.count; i++) {
                        if (Number(UID) === Number(importListModel.get(i).uuid)) {
                            importSideBar.view.currentIndex = i
                            importSideBar.view.currentItem.checked = true
                            break
                        }
                    }

                    autoShowSliderAnimation()
                }
            }

            onItemClicked: (uuid)=> {
               currentImportCustomIndex = importSideBar.indexFromUuid(uuid)
            }

            onItemCheckedChanged: (checked)=> {
                if (checked) {
                    GStatus.currentViewType = Album.Types.ViewCustomAlbum
                    GStatus.currentCustomAlbumUId = importListModel.get(index).uuid
                    GStatus.searchEditText = ""
                }
            }

            onItemRightClicked: {
                importMenu.popup()
            }

            onSigRemoveItem: {
                removeAlbumDialog.deleteType = 1
                removeAlbumDialog.show()
            }
        }

        // 用户自定义相册列表
        SideBarItem {
            id: customSideBar
            showTitle: false
            group: paneListGroup
            sideModel: customListModel

            onItemClicked: (uuid)=> {
                currentCustomIndex = customSideBar.indexFromUuid(uuid)
                GStatus.currentViewType = Album.Types.ViewCustomAlbum
                GStatus.currentCustomAlbumUId = customListModel.get(currentCustomIndex).uuid
                GStatus.searchEditText = ""
            }

            onItemRightClicked: {
                customMenu.popup()
            }

            onSigRemoveItem: {
                removeAlbumDialog.deleteType = 0
                removeAlbumDialog.show()
            }
        }
    }

    // 数据库监听-刷新设备列表
    Connections {
        target: albumControl
        function onSigMountsChange() {
            devicePaths = albumControl.getDevicePaths()
            if (devicePaths.length === 0 && GStatus.currentViewType === Album.Types.ViewDevice)
                backCollection()
        }
    }

    // 通过自定义相册列表创建相册后，导航到新相册所在行
    Connections {
        target: newAlbum
        function onSigCreateAlbumDone() {
            if (GStatus.currentViewType === Album.Types.ViewCustomAlbum) {
                for (var i = 0; i < customListModel.count; i++) {
                    if (Number(customListModel.get(i).uuid) === GStatus.currentCustomAlbumUId) {
                        customSideBar.view.currentIndex = i
                        customSideBar.view.currentItem.checked = true
                        customSideBar.view.currentItem.forceActiveFocus()
                        break
                    }
                }
            }
        }
    }

    // 系统相册菜单
    Menu {
        id: systemMenu

        //显示大图预览
        RightMenuItem {
            text: qsTr("Slide show")
            visible: albumPaths.length >0
            onTriggered: {
                stackControl.startMainSliderShow(albumPaths, 0)
            }
        }

        MenuSeparator {
        }

        RightMenuItem {
            text: qsTr("Export")
            visible:  albumPaths.length >0
            onTriggered: {
                albumControl.exportFolders(albumPaths,albumControl.getCustomAlbumByUid(GStatus.currentCustomAlbumUId))
            }
        }
    }

    // 自动导入相册菜单
    Menu {
        id: importMenu

        //显示大图预览
        RightMenuItem {
            text: qsTr("Slide show")
            visible: albumPaths.length > 0
            onTriggered: {
                stackControl.startMainSliderShow(albumPaths, 0)
            }
        }

        MenuSeparator {
            visible: albumPaths.length > 0
        }

        RightMenuItem {
            text: qsTr("Export")
            visible: albumPaths.length > 0
            onTriggered: {
                albumControl.exportFolders(albumPaths,albumControl.getCustomAlbumByUid(GStatus.currentCustomAlbumUId))
            }
        }

        RightMenuItem {
            text: qsTr("Delete")
            onTriggered: {
                removeAlbumDialog.deleteType = 1
                removeAlbumDialog.show()
            }
        }
    }

    //删除相册确认弹窗
    RemoveAlbumDialog {
        id: removeAlbumDialog
    }

    //删除相册执行函数
    function doDeleteAlbum(type) {
        if(type === 0) {
            // 删除自定义相册
            deleteCustomAlbum()
        } else if(type === 1) {
            // 删除自动导入相册
           deleteImportAlbum()
        }
    }

    Component.onCompleted: {
        removeAlbumDialog.sigDoRemoveAlbum.connect(doDeleteAlbum)
    }

    // 自定义相册菜单
    Menu {
        id: customMenu

        // 显示大图预览
        RightMenuItem {
            text: qsTr("Slide show")
            visible: albumPaths.length > 0
            onTriggered: {
                stackControl.startMainSliderShow(albumPaths, 0)
            }
        }

        // 新建相册
        RightMenuItem {
            text: qsTr("New album")
            visible: GStatus.currentCustomAlbumUId > 3 ? true : false
            onTriggered: {
                newAlbum.isChangeView = true
                newAlbum.setNormalEdit()
                newAlbum.show()
            }
        }

        // 重命名相册
        RightMenuItem {
            text: qsTr("Rename")
            visible: GStatus.currentCustomAlbumUId > 3 ? true : false
            onTriggered: {
                customSideBar.view.currentItem.rename()
            }
        }

        MenuSeparator {
        }

        // 导出相册
        RightMenuItem {
            text: qsTr("Export")
            visible: albumPaths.length > 0
            onTriggered: {
                albumControl.exportFolders(albumPaths,albumControl.getCustomAlbumByUid(GStatus.currentCustomAlbumUId))
            }
        }

        // 删除相册
        RightMenuItem {
            text: qsTr("Delete")
            visible: GStatus.currentCustomAlbumUId > 3 ? true : false
            onTriggered: {
                removeAlbumDialog.deleteType = 0
                removeAlbumDialog.show()
            }
        }
    }
}

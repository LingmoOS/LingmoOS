/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.3
import Deepin.Widgets 1.0
import AdjunctUploader 1.0
import "./SingleLineTipCreator.js" as IconTip

Rectangle {
    id: adjunctTray
    color: "#e8e8e8"
    width: parent.width

    property alias adjunctModel: adjunctView.model
    property var sysAdjunctModel: ListModel{}
    property bool available: false

    signal adjunctAdded()
    signal adjunctRemoved()
    signal sysAdjunctUploaded()

    function isUploaded(filePath){
        return AdjunctUploader.isInUploadedList(filePath)
    }

    function checkAdjunctState(){
        for (var i = 0; i < adjunctView.model.count; i ++){
            if (!adjunctView.model.get(i).uploadFinish) {
                available = false;
                return;
            }
        }
        available = true;
    }

    function getBucketUrl(filePath){
        return AdjunctUploader.getBucketUrl(filePath)
    }

    function getAdjunctList(){
        var tmpList = []
        for (var i = 0; i < adjunctView.model.count; i ++){
            var tmpPath = adjunctView.model.get(i).filePath
            tmpList.push({
                          "name": getFileNameByPath(tmpPath),
                             "url":adjunctView.model.get(i).bucketUrl,
                             "type": getMimeTypeByPath(tmpPath)
                         })
        }
        for (var i = 0; i < sysAdjunctModel.count; i ++){
            tmpList.push({
                             "name": sysAdjunctModel.get(i).name,
                             "url": sysAdjunctModel.get(i).url,
                             "type": sysAdjunctModel.get(i).type
                         })
        }

        return tmpList
    }

    function addAdjunct(filePath){
        if (isBackendGenerateResult(filePath)){
            AdjunctUploader.uploadAdjunct(filePath)
        }
        else if (getIndexFromModel(filePath) == -1){
            var fileUploaded = isUploaded(filePath)
            adjunctView.model.append({
                                         "showIconOnly": false,
                                         "filePath": filePath,
                                         "bucketUrl": fileUploaded ? getBucketUrl(filePath) : "",
                                         "loadPercent": 0,
                                         "iconPath": "images/add-adjunct.png",
                                         "uploadFinish": fileUploaded,
                                         "gotError": false
                                     })
            adjunctTray.adjunctAdded()

            if (!fileUploaded)
                AdjunctUploader.uploadAdjunct(filePath)
        }
    }

    function removeAdjunct(filePath){
        var tmpIndex = getIndexFromModel(filePath)
        if (tmpIndex != -1){
            adjunctView.model.remove(tmpIndex)
            adjunctTray.adjunctRemoved()

            AdjunctUploader.cancelUpload(filePath)
        }
        checkAdjunctState()
    }

    function getMimeTypeByPath(filePath){
        return AdjunctUploader.getMimeType(filePath)
    }

    function updateLoadPercent(filePath, percent){
        var tmpIndex = getIndexFromModel(filePath)
        if (tmpIndex != -1){
            adjunctView.model.setProperty(tmpIndex,"loadPercent",percent)
        }
    }

    function getIndexFromModel(filePath){
        for (var i = 0; i < adjunctView.model.count; i ++){
            if (adjunctView.model.get(i).filePath == filePath){
                return i
            }
        }
        return -1
    }

    function clearAllAdjunct(){
        adjunctView.model.clear()
    }

    function showAddIcon(count){
        for (var i = 0; i < count; i ++){
            adjunctView.model.append({
                                         "showIconOnly": true,
                                         "filePath": "/" + i,
                                         "loadPercent": 0,
                                         "iconPath": "images/add-adjunct.png",
                                         "uploadFinish": false,
                                         "gotError": false
                                     })
        }
    }

    function hideAddIcon(count){
        for (var i = count; i > 0; i --)
            adjunctView.model.remove(adjunctView.model.count - 1)
    }

    Connections {
        target: AdjunctUploader
        onUploadProgress: {
            updateLoadPercent(filePath,progress / 100)
        }
        onUploadFinish: {
            if (isBackendGenerateResult(filePath)){
                sysAdjunctModel.append({
                                           "name": getFileNameByPath(filePath),
                                           "url":getBucketUrl(filePath),
                                           "type": getMimeTypeByPath(filePath)
                                       })

                if (-- sysAdjunctCount == 0){
                    adjunctTray.sysAdjunctUploaded()
                }
            }
            else{
                var tmpIndex = getIndexFromModel(filePath)
                if (tmpIndex != -1){
                    adjunctView.model.setProperty(tmpIndex,"uploadFinish",true)
                    adjunctView.model.setProperty(tmpIndex,"bucketUrl",bucketUrl)
                }
            }
            checkAdjunctState()
        }
        onUploadFailed: {
            if (isBackendGenerateResult(filePath)){
                mainObject.clearSysAdjuncts(lastTarget)
            }

            var tmpIndex = getIndexFromModel(filePath)
            if (tmpIndex != -1){
                adjunctView.model.setProperty(tmpIndex,"gotError",true)
                adjunctView.model.setProperty(tmpIndex,"bucketUrl","")
            }
            checkAdjunctState()
        }
    }
    function getStringPixelSize(preText,fontPixelSize){
        var tmpText = calculateTextComponent.createObject(null, { "text": preText , "font.pixelSize": fontPixelSize})
        var width = tmpText.width
        return width
    }

    property var calculateTextComponent: Component {
        Text{visible: false}
    }

    GridView {
        id: adjunctView
        anchors.fill: parent
        width: parent.width
        height: parent.height
        cellWidth: parent.width / 6
        cellHeight: 52

        model: ListModel {}
        delegate: AdjunctItem {
            clip: true
            width: adjunctView.cellWidth
            height: adjunctView.cellHeight
            onDeleteAdjunct: {
                removeAdjunct(filePath)
                mainObject.removeAdjunct(filePath)
            }
            onRetryUpload: {
                var tmpIndex = getIndexFromModel(filePath)
                if (tmpIndex != -1){
                    adjunctView.model.setProperty(tmpIndex,"gotError",false)
                }

                AdjunctUploader.uploadAdjunct(filePath)
            }
            onEntered: {
                IconTip.pageX = pageX
                IconTip.pageY = pageY
                if (error){
                    IconTip.tipColor = "#ff8c03"
                    IconTip.toolTip = getFileNameByPath(file_path) + "\n" + dsTr("Upload failed, please retry.")
                }
                else{
                    IconTip.tipColor = "#FFFFFF"
                    IconTip.toolTip = getFileNameByPath(file_path)
                }
                var textLength = getStringPixelSize(getFileNameByPath(IconTip.toolTip),13)
                if (textLength < 200)
                    IconTip.pageWidth = textLength + 30
                else
                    IconTip.pageWidth = 200
                IconTip.pageHeight = Math.ceil(textLength/200) * 26

                IconTip.showTip()
            }
            onExited: {
                IconTip.destroyTip()
            }
        }
    }
}


/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

import QtQuick 2.1
import QtQuick.Window 2.1
import Deepin.Locale 1.0
import Deepin.Widgets 1.0
import DBus.Com.Deepin.Daemon.Display 1.0
import DBus.Com.Deepin.Daemon.Search 1.0
import DBus.Com.Deepin.Feedback 1.0

QtObject {
    id: root

    property color textActivedColor: DPalette.hoverColor
    property color textNormalColor: "#5e5e5e"
    property color bgActivedColor: "#5498ec"
    property color bgNormalColor: DPalette.hoverColor
    property color buttonBorderColor: "#d7d7d7"
    property color buttonBorderActiveColor: "#7bbefb"
    property color buttonBorderWarningColor: "#FF8F00"
    property color warningTipsColor: "#ffa048"
    property color inputDisableBgColor: "#fbfbfb"

    property int maxAdjunctCount: 6

    property var projectList: projectListModel.getValueList()
    property var projectNameList:projectListModel.getNameList()
    property var projectEnNameList:projectListModel.getEnNameList()
    property var bugzillaProjectList: projectListModel.getBugzillaProjectList()
    property var projectListModel: ListModel {
        function getValueList(){
            var tmpValueList = new Array()
            for (var i = 0; i < count; i ++){
                tmpValueList.push(get(i).Value)
            }

            return tmpValueList
        }

        function getNameList(){
            var tmpNameList = new Array()
            for (var i = 0; i < count; i ++){
                tmpNameList.push(dsTr(get(i).Name))
            }

            return tmpNameList
        }

        function getEnNameList(){
            var tmpNameList = new Array()
            for (var i = 0; i < count; i ++){
                tmpNameList.push(get(i).Name)
            }

            return tmpNameList
        }

        function getBugzillaProjectList(){
            var tmpProList = new Array()
            for (var i = 0; i < count; i ++){
                tmpProList.push(get(i).BugzillaProject)
            }
            return tmpProList
        }

        Component.onCompleted: {
            var tmpValue = unmarshalJSON(feedbackContent.GetCategories())
            for (var key in tmpValue){
                append(tmpValue[key])
            }
        }
    }

    property var displayId: Display {}
    property var screenSize: QtObject {
        property int x: displayId.primaryRect[0]
        property int y: displayId.primaryRect[1]
        property int width: displayId.primaryRect[2]
        property int height: displayId.primaryRect[3]
    }
    property var backendGenerateResult: []  // File path list
    property var feedbackContent: Feedback {}
    property var mainWindow: MainWindow {}
    property var dbusSearch: Search {}
    property var dsslocale: DLocale {
        domain: "deepin-feedback"

        Component.onCompleted: {
            console.log ("Language:", dsslocale.lang)
        }
    }


    function isBackendGenerateResult(fullPath) {
        return backendGenerateResult.indexOf(getFileNameByPath(fullPath)) >= 0;
    }

    function getFileNameByPath(fullPath) {
        return fullPath.replace(/^.*[\\\/]/, '')
    }

    function getProjectIDByName(name){
        var tmpIndex = projectNameList.indexOf(name)
        if (tmpIndex == -1)
            return ""
        else
            return projectList[tmpIndex]
    }

    function getProjectNameByID(id){
        var tmpIndex = projectList.indexOf(id)
        if (tmpIndex == -1)
            return ""
        else
            return projectNameList[tmpIndex]
    }

    function getBugzillaProjectByName(name){
        var tmpIndex = projectNameList.indexOf(name)
        if (tmpIndex == -1)
            return ""
        else
            return bugzillaProjectList[tmpIndex]
    }

    function getSupportAppList(){
        return projectListModel.getValueList()
    }

    function setReportContent(value){
        mainWindow.updateReportContentText(value)
    }

    function setAdjunctsPathList(list){
        mainWindow.updateAdjunctsPathList(list)
    }

    function setSimpleEntries(_feedbackType,_reportTitle,_email,_helpDeepin){
        mainWindow.updateSimpleEntries(_feedbackType,_reportTitle,_email,_helpDeepin)
    }

    function switchProject(target){
        mainWindow.switchProject(target)
    }

    function showMainWindow(){
        mainWindow.show()
    }

    function hideMainWindow(){
        mainWindow.hide()
    }

    function dsTr(s){
        return dsslocale.dsTr(s)
    }

    function marshalJSON(value) {
        var valueJSON = JSON.stringify(value);
        return valueJSON
    }

    function unmarshalJSON(valueJSON) {
        var value = JSON.parse(valueJSON)
        return value
    }

}

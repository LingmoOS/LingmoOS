/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

var iconTipComponent = Qt.createComponent("SingleLineTip.qml");
var iconTipPage;
var pageX = 0
var pageY = 0
var pageWidth = 0
var pageHeight = 36
var tipColor = "#ff8c03"
var toolTip = ""


function showTip()
{
    if (iconTipPage != undefined)
    {
        iconTipPage.x = pageX
        iconTipPage.y = pageY
        iconTipPage.width = pageWidth;
        iconTipPage.height = pageHeight
        iconTipPage.textColor = tipColor
        iconTipPage.toolTip = toolTip
        iconTipPage.destroyInterval = 200
        iconTipPage.showTipAtTop()
    }
    else
    {
        var iconTipComponent = Qt.createComponent("SingleLineTip.qml")
        if (iconTipComponent.status === Component.Ready)
            fnishCreate();
        else if (iconTipComponent.status === Component.Error)
        {
            console.warn ("Component Error:", iconTipComponent.errorString());
        }
        else{
            iconTipComponent.statusChanged.connect(fnishCreate);
        }
    }
}

function fnishCreate()
{
    if (iconTipComponent.status === Component.Ready)
    {
        iconTipPage = iconTipComponent.createObject(undefined);

        if (iconTipPage === null)
        {
            console.warn ("Component createObject failed");
        }
        else
        {
            iconTipPage.x = pageX
            iconTipPage.y = pageY
            iconTipPage.width = pageWidth;
            iconTipPage.height = pageHeight
            iconTipPage.textColor = tipColor
            iconTipPage.toolTip = toolTip
            iconTipPage.destroyInterval = 200
            iconTipPage.showTipAtTop()
        }
    }
    else if (iconTipComponent.status === Component.Error) {
        console.warn ("Error loading component:", iconTipComponent.errorString());
    }
}


function destroyTip()
{
    if (iconTipPage != undefined){
        iconTipPage.destroyInterval = -1
        iconTipPage.destroyTip()
    }
}

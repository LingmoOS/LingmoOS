// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dprintertanslator.h"

DPrinterTanslator::DPrinterTanslator()
{
}

void DPrinterTanslator::init()
{
    m_mapTrans.clear();

    //ColorMode_Combo
    addTranslate("ColorMode_Combo", "Color", tr("Color"));
    addTranslate("ColorMode_Combo", "Grayscale", tr("Grayscale"));
    addTranslate("ColorMode_Combo", "None", tr("Grayscale"));

    //OutputQuanlity_Combo
    addTranslate("OutputMode_Combo", "None", tr("None"));
    addTranslate("OutputMode_Combo", "Draft", tr("Draft"));
    addTranslate("OutputMode_Combo", "Normal", tr("Normal"));

    //PrintQuality_Combo
    addTranslate("PrintQuality_Combo", "Printout Quality", tr("Print Quality"));
    addTranslate("PrintQuality_Combo", "PrintQuality", tr("Print Quality"));

    //StpQuality_Combo
    addTranslate("StpQuality_Combo", "Print Quality", tr("Print Quality"));
    addTranslate("StpQuality_Combo", "Manual Control", tr("Manual"));
    addTranslate("StpQuality_Combo", "Draft", tr("Draft"));

    //PaperOrigin_Combo
    addTranslate("InputSlot_Combo", "Paper Source", tr("Paper Source"));
    addTranslate("InputSlot_Combo", "Media Source", tr("Paper Source"));
    addTranslate("InputSlot_Combo", "InputSlot", tr("Paper Source"));
    addTranslate("InputSlot_Combo", "Auto-Select", tr("Auto-Select"));
    addTranslate("InputSlot_Combo", "Automatic Selection", tr("Auto-Select"));
    addTranslate("InputSlot_Combo", "Manual Feeder", tr("Manual Feeder"));
    addTranslate("InputSlot_Combo", "Manual Feed", tr("Manual Feeder"));
    addTranslate("InputSlot_Combo", "Auto", tr("Auto"));
    addTranslate("InputSlot_Combo", "Manual", tr("Manual"));
    addTranslate("InputSlot_Combo", "Drawer 1", tr("Drawer 1"));
    addTranslate("InputSlot_Combo", "Drawer 2", tr("Drawer 2"));
    addTranslate("InputSlot_Combo", "Drawer 3", tr("Drawer 3"));
    addTranslate("InputSlot_Combo", "Drawer 4", tr("Drawer 4"));
    addTranslate("InputSlot_Combo", "Drawer 5", tr("Drawer 5"));
    addTranslate("InputSlot_Combo", "Envelope Feeder", tr("Envelope Feeder"));
    addTranslate("InputSlot_Combo", "Tray1", tr("Tray1"));
    addTranslate("InputSlot_Combo", "Unknown", tr("Unknown"));
    addTranslate("InputSlot_Combo", "None", tr("None"));

    //PaperType_Combo
    addTranslate("MediaType_Combo", "Media Type", tr("MediaType"));
    addTranslate("MediaType_Combo", "Auto", tr("Auto"));
    addTranslate("MediaType_Combo", "Plain Paper", tr("Plain Paper"));
    addTranslate("MediaType_Combo", "Recycled Paper", tr("Recycled Paper"));
    addTranslate("MediaType_Combo", "Color Paper", tr("Color Paper"));
    addTranslate("MediaType_Combo", "Bond Paper", tr("Bond Paper"));
    addTranslate("MediaType_Combo", "Heavy Paper 1", tr("Heavy Paper 1"));
    addTranslate("MediaType_Combo", "Heavy Paper 2", tr("Heavy Paper 2"));
    addTranslate("MediaType_Combo", "Heavy Paper 3", tr("Heavy Paper 3"));
    addTranslate("MediaType_Combo", "OHP", tr("OHP"));
    addTranslate("MediaType_Combo", "CLEARFILM", tr("OHP"));
    addTranslate("MediaType_Combo", "Labels", tr("Labels"));
    addTranslate("MediaType_Combo", "Envelope", tr("Envelope"));
    addTranslate("MediaType_Combo", "Photo Paper", tr("Photo Paper"));
    addTranslate("MediaType_Combo", "None", tr("None"));

    //PageSize_Combo
    addTranslate("PageSize_Combo", "Media Size", tr("PageSize"));
    addTranslate("PageSize_Combo", "PageSize", tr("PageSize"));
    addTranslate("PageSize_Combo", "PageSize", tr("PageSize"));
    addTranslate("PageSize_Combo", "Custom", tr("Custom"));

    //Duplex_Combo
    addTranslate("Duplex_Combo", "Duplex", tr("Duplex"));
    addTranslate("Duplex_Combo", "2-Sided Printing", tr("Duplex"));
    addTranslate("Duplex_Combo", "Double-Sided Printing", tr("Duplex"));
    addTranslate("Duplex_Combo", "DuplexTumble", tr("DuplexTumble"));
    addTranslate("Duplex_Combo", "DuplexNoTumble", tr("DuplexNoTumble"));
    addTranslate("Duplex_Combo", "ON (Long-edged Binding)", tr("ON (Long-edged Binding)"));
    addTranslate("Duplex_Combo", "ON (Short-edged Binding)", tr("ON (Short-edged Binding)"));
    addTranslate("Duplex_Combo", "OFF", tr("OFF"));
    addTranslate("Duplex_Combo", "None", tr("OFF"));

    //BindEdge_Combo
    addTranslate("BindEdge_Combo", "BindingEdge", tr("Binding Edge"));
    addTranslate("BindEdge_Combo", "None", tr("None"));
    addTranslate("BindEdge_Combo", "Left", tr("Left"));
    addTranslate("BindEdge_Combo", "Top", tr("Top"));
    addTranslate("BindEdge_Combo", "BindEdge", tr("Binding Edge"));

    //Orientation_Combo
    addTranslate("Orientation_Combo", "Portrait (no rotation)", tr("Portrait (no rotation)"));
    addTranslate("Orientation_Combo", "Landscape (90 degrees)", tr("Landscape (90 degrees)"));
    addTranslate("Orientation_Combo", "Reverse landscape (270 degrees)", tr("Reverse landscape (270 degrees)"));
    addTranslate("Orientation_Combo", "Reverse portrait (180 degrees)", tr("Reverse portrait (180 degrees)"));
    addTranslate("Orientation_Combo", "Auto", tr("Auto Rotation"));

    //PrintOrder_Combo
    addTranslate("PrintOrder_Combo", "Normal", tr("Normal"));
    addTranslate("PrintOrder_Combo", "Reverse", tr("Reverse"));

    //StapleLocation_Combo
    addTranslate("StapleLocation_Combo", "StapleLocation", tr("Staple Location"));
    addTranslate("StapleLocation_Combo", "Staple Location", tr("Staple Location"));
    addTranslate("StapleLocation_Combo", "None", tr("None"));
    addTranslate("StapleLocation_Combo", "Bind", tr("Bind"));
    addTranslate("StapleLocation_Combo", "Bind (none)", tr("Bind (none)"));
    addTranslate("StapleLocation_Combo", "Bind (bottom)", tr("Bind (bottom)"));
    addTranslate("StapleLocation_Combo", "Bind (left)", tr("Bind (left)"));
    addTranslate("StapleLocation_Combo", "Bind (right)", tr("Bind (right)"));
    addTranslate("StapleLocation_Combo", "Bind (top)", tr("Bind (top)"));

    //Resolution_Combo
    addTranslate("Resolution_Combo", "None", tr("None"));
    addTranslate("Resolution_Combo", "Resolution", tr("Resolution"));
    addTranslate("Resolution_Combo", "Output Resolution", tr("Resolution"));
}

void DPrinterTanslator::addTranslate(const QString &strContext, const QString &strKey, const QString &strValue)
{
    QMap<QString, QString> mapNode = m_mapTrans.value(strContext);

    if (mapNode.isEmpty()) {
        mapNode.insert(strKey, strValue);
        m_mapTrans.insert(strContext, mapNode);
    } else {
        mapNode[strKey] = strValue;
        m_mapTrans.insert(strContext, mapNode);
    }
}

QString DPrinterTanslator::translateLocal(const QString &strContext, const QString &strKey, const QString &strDefault)
{
    QMap<QString, QString> mapNode = m_mapTrans.value(strContext);

    if (mapNode.isEmpty()) {
        return strDefault;
    }

    QString strValue = mapNode.value(strKey);
    return strValue.isEmpty() ? strDefault : strValue;
}

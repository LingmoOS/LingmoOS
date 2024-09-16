// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "getdlnaxmlvalue.h"



GetDlnaXmlValue::GetDlnaXmlValue(QByteArray data)
{
    doc = new QDomDocument;
    doc->setContent(data);
}

GetDlnaXmlValue::~GetDlnaXmlValue()
{
    if(doc) {
       delete doc;
        doc = NULL;
    }
}

QString GetDlnaXmlValue::getValueByPath(QString sPath)
{
    QStringList sList = sPath.split("/");
    QDomElement elm = getElmByPath(sList);
    return getElmText(elm).trimmed();
}
/**
 * @brief getValueByPathValue 根据xml节点的值查找当前节点下的指定节点的值
 * @param sPath 查找xml节点路径
 * @param sValue 查找xml节点值
 * @param sElm 函数需要返回节点的值需要
 */
QString GetDlnaXmlValue::getValueByPathValue(QString sPath, QString sValue, QString sElm)
{
    QStringList sList = sPath.split("/");
    QDomElement elm = getElmByPath(sList);
    QDomElement childElm = getElmByPath(elm, sValue, sElm);
    return getElmText(childElm).trimmed();
}
/**
 * @brief getElmByPath 获取路径的节点
 * @param sList 查找xml节点路径
 */
QDomElement GetDlnaXmlValue::getElmByPath(QStringList sList)
{
    QDomNode node = doc->firstChild();
    QDomElement elm = node.toElement();
    if(elm.isNull())
    {
        node = node.nextSibling();
        elm = node.toElement();
        if(elm.isNull())
            return QDomElement();
    }
    foreach(QString tagName, sList) {
        elm = elm.firstChildElement(tagName);
        if(elm.isNull())
        {
            return QDomElement();
        }
    }
    return elm;
}
/**
 * @brief getElmByPath 获取路径的节点
 * @param elm 查找xml节点路径
 * @param sValue 查找xml节点路径
 * @param sElm 查找xml节点路径
 */
QDomElement GetDlnaXmlValue::getElmByPath(QDomElement childElm, QString sValue, QString sElmText)
{
    if(childElm.isNull()) return QDomElement();
    QStringList sList = sValue.split("=");
    if(sList.size() != 2) return QDomElement();
    QDomElement elm = childElm.firstChildElement("service");;
    while(!elm.isNull()) {
        QDomElement matchElm = elm.firstChildElement(sList[0]);
        QString sText = getElmText(matchElm);
        if(sText == sList[1]) {
            return elm.firstChildElement(sElmText);
        }
        elm = elm.nextSibling().toElement();
    }
    return QDomElement();
}
/**
 * @brief getElmText 获取节点的值
 * @param elm 查找xml节点路径
 */
QString GetDlnaXmlValue::getElmText(QDomElement elm)
{
    if(!elm.isNull()) {
        return elm.text();
    }
    return "";
}


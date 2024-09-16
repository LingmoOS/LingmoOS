// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GETDLNAXMLVALUE_H
#define GETDLNAXMLVALUE_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QStringList>

class GetDlnaXmlValue : public QObject
{
    Q_OBJECT
public:
    GetDlnaXmlValue(QByteArray data = "");
    ~GetDlnaXmlValue();
    /**
     * @brief getValueByPath 获取xml节点路径值
     * @param sPath xml节点路径
     */
    QString getValueByPath(QString sPath);
    /**
     * @brief getValueByPathValue 根据xml节点的值查找当前节点下的指定节点的值
     * @param sPath 查找xml节点路径
     * @param sValue 查找xml节点值
     * @param sElm 函数需要返回节点的值需要
     */
    QString getValueByPathValue(QString sPath, QString sValue, QString sElm);

private:
    QDomDocument *doc;
private:
    /**
     * @brief getElmByPath 获取路径的节点
     * @param sList 查找xml节点路径
     */
    QDomElement getElmByPath(QStringList sList);
    //查找elm 下 属性值为sValue的下的sElm元素的值
    //"./device/serviceList/service/"
    //"[serviceType='{0}']/controlURL"
    /**
     * @brief getElmByPath 获取路径的节点
     * @param elm 查找xml节点路径
     * @param sValue 查找xml节点路径
     * @param sElm 查找xml节点路径
     */
    QDomElement getElmByPath(QDomElement elm, QString sValue, QString sElm);
    /**
     * @brief getElmText 获取节点的值
     * @param elm 查找xml节点路径
     */
    QString getElmText(QDomElement elm);
};

#endif // GETDLNAXMLVALUE_H

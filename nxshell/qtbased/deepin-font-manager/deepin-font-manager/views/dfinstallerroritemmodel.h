// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFINSTALLERRORITEMMODEL_H
#define DFINSTALLERRORITEMMODEL_H

#include <QMetaType>
#include <QString>

/*************************************************************************
 <Struct>        DFInstallErrorItemModel
 <Description>   字体信息结构体
 <Attribution>
    <Attr1>bChecked              Description:是否勾选
    <Attr2>bSelectable           Description:是否可选
    <Attr3>bSystemFont           Description:是否为系统字体
    <Attr4>bIsNormalUserFont     Description:是否为正常字体
    <Attr5>strFontFileName       Description:字体文件名属性信息
    <Attr6>strFontFilePath       Description:字体路径属性信息
    <Attr7>strFontInstallStatus  Description:安装状态，是否为已安装
 <Note>          null
*************************************************************************/
struct DFInstallErrorItemModel {
    bool bChecked;
    bool bSelectable;
    bool bSystemFont;
    bool bIsNormalUserFont;//SP3--安装验证页面，回车取消/选中(539)--设置字体状态
    QString strFontFileName;
    QString strFontFilePath;
    QString strFontInstallStatus;

    DFInstallErrorItemModel()
        : bChecked(false)
        , bSelectable(false)
        , bSystemFont(false)
        , bIsNormalUserFont(false) //SP3--安装验证页面，回车取消/选中(539)--初始化字体状态
        , strFontFileName("")
        , strFontFilePath("")
        , strFontInstallStatus("")

    {
    }
};

Q_DECLARE_METATYPE(DFInstallErrorItemModel)

#endif  // DFINSTALLERRORITEMMODEL_H

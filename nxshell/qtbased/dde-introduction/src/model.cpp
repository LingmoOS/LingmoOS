// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model.h"

//主题图标排序函数
bool iconSort(const IconStruct &icon1, const IconStruct &icon2) {
    QCollator qc;
    return qc.compare(icon1.Id, icon2.Id) < 0;
}

Model *Model::Instance()
{
    static Model * instance = new Model;
    return instance;
}

/*******************************************************************************
 1. @函数:    currentIcon
 2. @作者:
 3. @日期:    2020-12-10
 4. @说明:    获取当前主题图标数据，如果有返回找到的数据，反之，返回空的数据
*******************************************************************************/
IconStruct Model::currentIcon()
{
    for (const IconStruct &tmp : m_iconList) {
        if (tmp.Id == m_currentIcon) {
            return tmp;
        }
    }

    return IconStruct();
}

/*******************************************************************************
 1. @函数:    addIcon
 2. @作者:
 3. @日期:    2020-12-10
 4. @说明:    添加图标槽
*******************************************************************************/
void Model::addIcon(const IconStruct &icon)
{
    if (m_iconList.contains(icon)) {
        return;
    }

    //添加到list
    m_iconList << icon;

    //排序
    std::sort(m_iconList.begin(), m_iconList.end(), iconSort);

    emit iconAdded(icon);

    if (icon.Id == m_currentIcon) {
        emit iconChanged(icon);
    }
}

/*******************************************************************************
 1. @函数:    removeIcon
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    删除图标槽函数
*******************************************************************************/
void Model::removeIcon(const IconStruct &icon)
{
    Q_ASSERT(m_iconList.contains(icon));

    m_iconList.removeOne(icon);

    emit iconRemoved(icon);
}

/*******************************************************************************
 1. @函数:    setCurrentIcon
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    设置当前图标名
*******************************************************************************/
void Model::setCurrentIcon(const QString &icon)
{
    if (icon == m_currentIcon) {
        return;
    }

    m_currentIcon = icon;

    for (const IconStruct &tmp : m_iconList) {
        if (tmp.Id == icon) {
            emit iconChanged(tmp);
        }
    }
}

Model::Model(QObject *parent)
    : QObject(parent)
{

}

/*******************************************************************************
 1. @函数:    setDesktopMode
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    设置桌面样式类型
*******************************************************************************/
void Model::setDesktopMode(DesktopMode desktopMode)
{
    if (m_desktopMode == desktopMode) {
        return;
    }

    m_desktopMode = desktopMode;

    emit desktopModeChanged(desktopMode);
}

/*******************************************************************************
 1. @函数:    setWmType
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    设置运行模式类型
*******************************************************************************/
void Model::setWmType(WMType wmType)
{
    if (m_wmType == wmType) {
        return;
    }

    m_wmType = wmType;

    emit wmTypeChanged(wmType);
}

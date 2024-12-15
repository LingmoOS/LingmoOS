// Copyright (C) 2021 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menuitem.h"

MenuItem::MenuItem(int iMenuId, const QString &strMenuName, bool bIsChecked /* = false */)
    : m_iMenuId(iMenuId)
    , m_strMenuName(strMenuName)
    , m_bIsChecked(bIsChecked)
{
}

MenuItem::MenuItem()
{
    m_iMenuId = -1;
    m_strMenuName = QString("");
    m_bIsChecked = false;
}

MenuItem::MenuItem(const MenuItem &menuItem)
{
    m_iMenuId = menuItem.GetMenuId();
    m_strMenuName = menuItem.GetMenuName();
    m_bIsChecked = menuItem.GetIsChecked();
}

MenuItem::~MenuItem() { }

int MenuItem::GetMenuId() const
{
    return m_iMenuId;
}

void MenuItem::SetMenuId(int iValue)
{
    m_iMenuId = iValue;
}

QString MenuItem::GetMenuName() const
{
    return m_strMenuName;
}

void MenuItem::SetMenuName(const QString &strValue)
{
    m_strMenuName = strValue;
}

// 返回当前控件是否被选中
bool MenuItem::GetIsChecked() const
{
    return m_bIsChecked;
}

void MenuItem::SetIsChecked(bool bValue)
{
    m_bIsChecked = bValue;
}

// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "tabletconfig.h"

//默认不为平板模式
bool TabletConfig::m_isTablet = false;
TabletConfig::TabletConfig(QObject *parent)
    : QObject(parent)
{
}

bool TabletConfig::isTablet()
{
    return m_isTablet;
}

void TabletConfig::setIsTablet(bool isTablet)
{
    m_isTablet = isTablet;
}

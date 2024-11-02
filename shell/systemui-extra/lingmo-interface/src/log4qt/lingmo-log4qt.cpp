/*
 * Copyright (C) 2021 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "lingmo-log4qt.h"
#include <QDebug>

#include "lingmo-logconfigurator.h"

// 初始化lingmo-log4qt库，传入application名称
int initLingmoUILog4qt(QString strAppName)
{
    if (strAppName.isEmpty()){
        return -1;
    }
    // 检查纠正名称格式 小写+'-'+数字
    strAppName = strAppName.toLower();
    // 非数字和小写字母以'-'代替
    strAppName.replace(QRegExp("[^a-z0-9]+"),"-");
    // 多个'-'以一个'-'代替
    strAppName.replace(QRegExp("[-]+"),"-");
    // 去掉头部的非字母串和尾部的'-'’
    strAppName.replace(QRegExp("(^[^a-z]+|-$)"),"");
    // 初始化log4qt配置
    return LingmoUILog4qtConfig::instance()->init(strAppName);
}

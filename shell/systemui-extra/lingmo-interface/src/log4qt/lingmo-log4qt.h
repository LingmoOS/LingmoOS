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

#ifndef __LINGMOLOG4QT_H__
#define __LINGMOLOG4QT_H__

#include <QDebug>

//输出debug级别日志
#define KyDebug QMessageLogger(__FILE__, __LINE__, __FUNCTION__).debug

//输出info级别日志
#define KyInfo QMessageLogger(__FILE__, __LINE__, __FUNCTION__).info

//输出warning级别日志
#define KyWarning QMessageLogger(__FILE__, __LINE__, __FUNCTION__).warning

//输出critical级别日志
#define KyCritical QMessageLogger(__FILE__, __LINE__, __FUNCTION__).critical

//输出fatal级别日志
#define KyFatal QMessageLogger(__FILE__, __LINE__, __FUNCTION__).fatal

// 初始化lingmo-log4qt库，传入application名称
int initLingmoUILog4qt(QString strAppName);

#endif // __LINGMOLOG4QT_H__

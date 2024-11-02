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

#ifndef __LINGMO_LOGMACROS__
#define __LINGMO_LOGMACROS__

#include <QtCore/qglobal.h>

#if defined(LIBLINGMOLOG4QT_LIBRARY)
#  define LIBLINGMOLOG4QT_EXPORT Q_DECL_EXPORT
#else
#  define LIBLINGMOLOG4QT_EXPORT Q_DECL_IMPORT
#endif

// 日志文件相对home目录路径
#define LINGMOLOG4QT_ROOTPATH  "/.log/"

// gsettings相关相关属性
#define LINGMOLOG4QT_SETTINGS "org.lingmo.lingmo-log4qt"
#define LINGMOLOG4QT_SETTINGS_RESET "log4jReset"
#define LINGMOLOG4QT_SETTINGS_DEBUG "log4jDebug"
#define LINGMOLOG4QT_SETTINGS_HANDLEQT "log4jHandleqtmessages"
#define LINGMOLOG4QT_SETTINGS_ROOTLOGGER "log4jRootlogger"
#define LINGMOLOG4QT_SETTINGS_DAILY_DATEPATTERN "log4jAppenderDailyDatepattern"
#define LINGMOLOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN "log4jAppenderDailyLayoutConversionpattern"
#define LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME "delaytime"
#define LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT "maxfilecount"
#define LINGMOLOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE "maxfilesize"

// log4qt相关属性
#define LINGMOLOG4QT_RESET "log4j.reset"                                                      //是否重置所有配置，恢复全局设置默认值
#define LINGMOLOG4QT_DEBUG "log4j.Debug"                                                      //设置日志sdk内部记录器级别
#define LINGMOLOG4QT_HANDLEQT "log4j.handleQtMessages"                                        //设置是否监听QDebug输出的字符串
#define LINGMOLOG4QT_ROOTLOGGER "log4j.rootLogger"                                            //设置根Logger的输出log等级和输出目的地
#define LINGMOLOG4QT_DAILY_DATEPATTERN "log4j.appender.daily.datePattern"                     //记录器文件名日期后缀 按天
#define LINGMOLOG4QT_DAILY_CONVERSIONPATTERN "log4j.appender.daily.layout.conversionPattern"  //记录器日志输出格式

#endif // __LINGMO_LOGMACROS__

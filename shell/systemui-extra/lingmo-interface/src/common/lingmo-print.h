/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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


#ifndef __LINGMOPRINT_H__
#define __LINGMOPRINT_H__

#include<stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define PRINT_LEVEL_EMERG	0
#define PRINT_LEVEL_ALERT	1
#define PRINT_LEVEL_CRIT	2
#define PRINT_LEVEL_ERROR	3	// 错误信息
#define PRINT_LEVEL_WARN	4	// 警告信息
#define PRINT_LEVEL_NOTICE	5	// 提示信息
#define PRINT_LEVEL_INFO	6	// 运行时信息
#define PRINT_LEVEL_DEBUG	7	// 调试信息

//#define PRINT_LEVEL_DEFAULT	PRINT_LEVEL_NOTICE	// 默认输出信息等级
#define PRINT_LEVEL_DEFAULT	PRINT_LEVEL_DEBUG
//#define PRINT_LEVEL_ENV		"PRINT_LEVEL"	// 控制信息等级的环境变量
#define PRINT_LEVEL_COLOR	"PRINT_COLOR"	// 控制信息颜色的环境变量

#define PRINT_COLOR_BLACK	"\033[01;30m"
#define PRINT_COLOR_RED	"\033[01;31m"
#define PRINT_COLOR_GREEN	"\033[01;32m"
#define PRINT_COLOR_YELLOW	"\033[01;33m"
#define PRINT_COLOR_BLUE	"\033[01;34m"
#define PRINT_COLOR_PURPLE	"\033[01;35m"
#define PRINT_COLOR_CYAN	"\033[01;36m"
#define PRINT_COLOR_WHITE	"\033[01;36m"
#define PRINT_COLOR_END	"\033[0m"

int vprint_by_level(int print_level, FILE * __restrict stream,
					   char * __restrict format, va_list args);
int print_by_level(int print_level, FILE * stream, char * format, ...);
int print_debug(char * format, ...);
int print_info(char * format, ...);
int print_notice(char * format, ...);
int print_warning(char * format, ...);
int print_error(char * format, ...);

#ifdef  __cplusplus
}
#endif

#endif

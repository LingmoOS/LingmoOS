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

#ifndef __LINGMOINI_H__
#define __LINGMOINI_H__

#include <stdio.h>
#include <string.h>

#ifdef  __cplusplus
extern "C" {
#endif

// 去除尾部空格
char *strstripr(char *str, int containQuot);

// 去除头部空格
char *strstripl(char *str, int containQuot);

// 去除前后空格
char *strstrip(char *str, int containQuot);

#ifdef  __cplusplus
}
#endif

#endif //__LINGMOINI_H__
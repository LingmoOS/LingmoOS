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

#include "lingmo-ini.h"

#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//去除尾部空白字符 包括\t \n \r  
/*
标准的空白字符包括：
' '     (0x20)    space (SPC) 空格符
'\t'    (0x09)    horizontal tab (TAB) 水平制表符    
'\n'    (0x0a)    newline (LF) 换行符
'\v'    (0x0b)    vertical tab (VT) 垂直制表符
'\f'    (0x0c)    feed (FF) 换页符
'\r'    (0x0d)    carriage return (CR) 回车符
//windows \r\n linux \n mac \r
*/ 
char *strstripr(char *str, int containQuot)
{
    if(str == NULL || *str == '\0') {
        return str;
    }
    int len = strlen(str);
    char *p = str + len - 1;
    if (containQuot) {
        while(p >= str && (isspace(*p) || *p == '\"')) {
            *p = '\0'; --p;
        }
    } else {
        while(p >= str && isspace(*p)) {
            *p = '\0'; --p;
        }
    }
    return str;
}

//去除首部空格 
char *strstripl(char *str, int containQuot)
{
    if(str == NULL || *str == '\0') {
        return str;
    }
    int len = 0;
    char *p = str;
    if (containQuot) {
        while(*p != '\0' && (isspace(*p) || *p == '\"')) { 
            ++p; ++len;
        }
    } else {
        while(*p != '\0' && isspace(*p)) { 
            ++p; ++len;
        }
    }
    memmove(str, p, strlen(str) - len + 1);
    return str;
}

//去除首尾空格
char *strstrip(char *str, int containQuot)
{
    str = strstripr(str, containQuot);
    str = strstripl(str, containQuot);
    return str;
}

/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */


#ifndef LINGMOSDK_BASE_UTILS_STREXT_H__
#define LINGMOSDK_BASE_UTILS_STREXT_H__


/** @defgroup C语言字符串扩展模块
  * @{
  */


/**
 * @file cstring-extension.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief LINGMOSDK C语言字符串操作扩展
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief 对给定的字符串进行strip操作，删减字符串前后的指定字符；注意该操作会修改原字符串
 * 
 * @param str 需要进行strip的字符串指针
 * @param ch 需要删除的字符
 */
static inline void strstrip(char *str, char ch)
{
    if (strlen(str) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && *startPos == ch)
        startPos++;
    if (*startPos == '\0')
    {
        str[0] = 0;
        return;
    }

    char *endPos = str + strlen(str) - 1;
    while (endPos != str && *endPos == ch)
        endPos --;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

/**
 * @brief 删除给定字符串前后的空格、制表符、换行符，注意该操作会修改原字符串
 * 
 * @param str 需要进行strip操作的字符串指针
 */
static inline void strstripspace(char *str)
{
    if (strlen(str) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && isspace(*startPos))
        startPos++;
    if (*startPos == '\0')
    {
        str[0] = 0;
        return;
    }

    char *endPos = str + strlen(str) - 1;
    while (endPos != str && isspace(*endPos))
        endPos --;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

/**
 * @brief 删除给定字符串前后的空格和水平制表符（tab），注意该操作会修改原字符串
 * 
 * @param str 需要修改的字符串指针
 */
static inline void strstripblank(char *str)
{
    if (strlen(str) == 0)
        return;
    char *startPos = str;
    while (*startPos != '\0' && isblank(*startPos))
        startPos++;
    if (*startPos == '\0')
    {
        str[0] = 0;
        return;
    }

    char *endPos = str + strlen(str) - 1;
    while (endPos != str && isblank(*endPos))
        endPos --;

    memmove(str, startPos, endPos - startPos + 1);
    *(str + (endPos - startPos) + 1) = 0;
}

/**
 * @brief 跳过字符串前的所有空格、制表符、换行符；该操作不会修改原字符串
 * 
 * @param p 指向原字符串的指针
 * @return const char* 指向跳过space后的字符串指针
 */
static inline const char *strskipspace(const char *p)
{
    while (isspace(*p))
		++p;
	return p;
}

/**
 * @brief 跳过字符串前的所有空格和水平制表符（tab）；该操作不会修改原字符串
 * 
 * @param p 指向原字符串的指针
 * @return const char* 指向跳过space后的字符串指针
 */
static inline const char *strskipblank(const char *p)
{
	while (isblank(*p))
		++p;
	return p;
}

/**
 * @brief 将字符串中的所有小写字母转化为大写字母；注意该操作会修改原字符串
 * 
 * @param str 需要操作的字符串指针
 */
static inline void str2upper(char *str)
{
    char *pos = str;
    while (*pos)
    {
        if (isalpha(*pos))
            *pos &= 0xDF;
        pos ++;
    }
}

/**
 * @brief 将字符串中的所有大写字母转化为小写字母；注意该操作会修改原字符串
 * 
 * @param str 需要操作的字符串指针
 */
static inline void str2lower(char *str)
{
    char *pos = str;
    while (*pos)
    {
        if (isalpha(*pos))
            *pos |= 0x20;
        pos ++;
    }
}

/**
 * @brief 在给定的字符串中查找给定字符第一次出现的位置；计数从0开始
 * 
 * @param str 原字符串
 * @param ch 需要查找的字符
 * @return int 第一次出现的位置，若未找到给定的字符，则返回-1
 */
static inline int strfirstof(const char* str, char ch)
{
    const char* pos = str;
    while (*pos)
    {
        if (*pos == ch)
            return pos - str;
        
        pos ++;
    }
    return -1;
}

/**
 * @brief 在给定的字符串中查找给定字符最后一次出现的位置；计数从0开始
 * 
 * @param str 原字符串
 * @param ch 需要查找的字符
 * @return int 最后一次出现的位置，若未找到给定的字符，则返回-1
 */
static inline int strlastof(const char* str, char ch)
{
    const char* pos = str;
    while (*pos)
        pos ++;
    pos --;
    while (pos != str)
    {
        if (*pos == ch)
            return pos - str;
	pos --;
    }
    return -1;
}

/**
 * @brief 判断str是否以prefix开头；注意该函数区分大小写
 * 
 * @param str 原字符串
 * @param prefix 需要匹配的字符串前缀
 * @return int 若str以prefix开头，则返回0；否则返回1
 */
static inline int strstartswith(const char *str, const char *prefix)
{
    size_t sz = prefix ? strlen(prefix) : 0;

    if (str && sz && strncmp(str, prefix, sz) == 0)
        return 0;
    return 1;
}

/**
 * @brief 判断str是否以prefix开头；不区分大小写
 * 
 * @param str 原字符串
 * @param prefix 需要匹配的字符串前缀
 * @return int 若str以prefix开头，则返回0；否则返回1
 */
static inline int strstartswith_nocase(const char *str, const char *prefix)
{
    size_t sz = prefix ? strlen(prefix) : 0;

    if (str && sz && strncasecmp(str, prefix, sz) == 0)
        return 0;
    return 1;
}

/**
 * @brief 判断str是否以postfix结尾；注意该函数区分大小写
 * 
 * @param str 原字符串
 * @param postfix 需要匹配的字符串后缀
 * @return int 若str以postfix结尾，则返回0；否则返回1
 */
static inline int strendwith(const char *str, const char *postfix)
{
    size_t sl = str ? strlen(str) : 0;
	size_t pl = postfix ? strlen(postfix) : 0;

	if (pl == 0)
		return 0;
	if (sl < pl)
		return 1;
	if (memcmp(str + sl - pl, postfix, pl) != 0)
		return 1;
	return 0;   
}

static inline int strendwith_nocase(const char *str, const char *postfix)
{
    return 0;
}

/**
 * @brief 统计给定字符在字符串中出现的次数
 * 
 * @param str 原字符串
 * @param ch 需要统计的字符
 * @return size_t 字符出现的次数
 */
static inline size_t strcounts(const char *str, char ch)
{
    const char *p = str;
    int counts = 0;
    while (*p)
    {
        if (*p == ch)
            counts ++;
        p ++;
    }

    return counts;
}

/**
 * @brief 对原字符串以给定的分隔符进行分割，注意该函数会修改原字符串
 * 
 * @param str 需要分割的字符串
 * @param delim 分隔符
 * @return char** 分割后的字符串列表，以NULL结尾。存储分割后所有字符串的字符串列表本身是由alloc申请的内存，因此当使用
 * 完成后应当被free；而分割出来的各个字符串不是申请的内存，而是分别指向了原字符串中的特定位置，因此他们不需要被分别free
 */
static inline char** strsplit(char *str, char delim)
{
    size_t size = strcounts(str, delim) + 1;
    char **res = (char **)calloc(1, sizeof(char *) * (size + 1));
	if (!res)
		return NULL;
    if (size < 2)
    {
        res[0] = str;
        return res;
    }
    char *leftstr;
    res[0] = strtok_r(str, &delim, &leftstr);
    for (size_t i = 1; i < size; i ++)
    {
        res[i] = strtok_r(NULL, &delim, &leftstr);
    }

    return res;
}

#ifdef __cplusplus
}
#endif

#endif
/**
  * @}
  */

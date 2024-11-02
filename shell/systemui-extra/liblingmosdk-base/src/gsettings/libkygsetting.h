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
 * Authors: shaozhimin <shaozhimin@kylinos.cn>
 *
 */

#ifndef	LINGMOSDK_BASE_GSETTINGS_H__
#define	LINGMOSDK_BASE_GSETTINGS_H__


/** @defgroup Gsettings
  * @{
  */


/**
 * @file libkygsetting.h
 * @author zhouhuazhi (zhouhuazhi@kylinos.cn)
 * @brief LINGMOSDK C语言Gsettings模块
 * @version 0.1
 * @date 2022-10-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */



#ifdef	__cplusplus
extern "C"
{

#endif
/**
 * @brief 设置gesettings指定key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @param format key 的类型，需要是 GVariant 支持的类型，除了可以使用基本的类型外，也可按照 GVariant 的方式组合类型
 * @return int 成功返回1，失败返回0
 */
extern int kdk_gsettings_set(const char *schema_id, const char *key, const char *format, ...);

/**
 * @brief 重置gesettings指定key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @return int 成功返回1，失败返回0
 */
extern int kdk_settings_reset(const char *schema_id, const char *key);

/**
 * @brief 设置gesettings指定string类型key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @param value key的值
 * @return int 成功返回1，失败返回0
 */
extern int kdk_settings_set_string(const char *schema_id, const char *key, const char *value);

/**
 * @brief 设置gesettings指定int类型key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @param value key的值
 * @return int 成功返回1，失败返回0
 */
extern int kdk_settings_set_int(const char *schema_id, const char *key, int value);

/**
 * @brief 获取gesettings指定key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @param format key 的类型，需要是 GVariant 支持的类型，除了可以使用基本的类型外，也可按照 GVariant 的方式组合类型
 * @return GVariant key的值
 */
void* kdk_gsettings_get(const char *schema_id, const char *key, const char *format, ...);

/**
 * @brief 获取gesettings指定string类型key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @return char* 成功返回key的值，失败返回NULL
 */
extern char* kdk_settings_get_string(const char *schema_id, const char *key);

/**
 * @brief 获取gesettings指定int类型key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @return int 成功返回key的值，失败返回0
 */
extern int kdk_settings_get_int(const char *schema_id, const char *key);

/**
 * @brief 获取gesettings指定double类型key值
 * 
 * @param schema_id schema中的id 
 * @param key key的名称
 * @return double 成功返回key的值，失败返回0
 */
extern double kdk_settings_get_double(const char *schema_id, const char *key);

#ifdef	__cplusplus
}
#endif

#endif	//LINGMOSDK_BASE_GSETTINGS_H__

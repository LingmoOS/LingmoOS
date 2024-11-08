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


#ifndef LINGMOSDK_BASE_CONFIGURE_H__
#define LINGMOSDK_BASE_CONFIGURE_H__


/** @defgroup 配置文件模块
  * @{
  */


/**
 * @file libkyconf.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief LINGMOSDK配置文件处理库，支持标准格式、XML(未实现)、JSON(未实现)的配置文件处理，包括配置文件的读与写操作
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <sys/types.h>
#include "sdkmarcos.h"



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 初始化配置文件
 * 
 * @param confpath 配置文件的路径
 * @return int 成功返回句柄号（非负值），失败返回错误码（负值）
 */
extern int kdk_conf_init(const char* confpath);

/**
 * @brief 销毁指定的配置文件句柄
 * 
 * @param id 由kdk_conf_init返回的配置文件句柄
 */
extern void kdk_conf_destroy(int id);

/**
 * @brief 重新载入配置文件
 * 
 * @param id 由kdk_conf_init返回的配置文件句柄
 * @return int 成功返回0，失败返回错误码
 */
extern int kdk_conf_reload(int id);

/**
 * @brief [未实现] 启用配置文件自动刷新功能，自动刷新会在配置文件被修改时，自动将所有的配置项重新载入内存；下次再获取某个值时就是最新的配置值了
 * 
 * @param id 由kdk_conf_init返回的配置文件句柄
 */
extern void kdk_conf_enable_autoreload(int id);

/**
 * @brief [未实现] 禁用配置文件自动刷新功能
 * 
 * @param id 由kdk_conf_init返回的配置文件句柄
 */
extern void kdk_conf_disable_autoreload(int id);

/**
 * @brief 获取指定配置项的值
 * 
 * @param id 由kdk_conf_init返回的配置文件句柄
 * @param group key所在的组名称
 * @param key 配置项名称
 * @return const char* 配置项所拥有的值，若key不存在，则返回一个空字符串
 */
extern const char* kdk_conf_get_value(int id, const char* group, const char* key);

/**
 * @brief [未实现] 设置对应key的值
 * 
 * @param id 由kdk_conf_init返回的配置文件句柄
 * @param group key所对应的组名称
 * @param key 需要修改的key的名称，若key原来不存在于该Group中，则会自动添加一个key
 * @param value 需要修改为的值
 * @return int 成功返回0，失败返回错误码
 */
extern int kdk_conf_set_value(int id, const char* group, const char* key, const char* value);

/**
 * @brief 列举id对应配置文件的指定Group下的key值，结尾以NULL指针表示
 * 
 * @param id 由kdk_conf_init返回的句柄值
 * @param group 需要列举的Group名称
 * @return const char** const 以NULL结尾的字符串列表，每个字符串都是一个key名称，字符串列表本身是由alloc分配的内存，需要被free释放；字符串不需要释放
 */
extern char** const kdk_conf_list_key(int id, const char* group);

/**
 * @brief 列举id对应配置文件的所有Group，结尾以NULL指针表示
 * 
 * @param id 由kdk_conf_init返回的句柄值
 * @return const char** const 以NULL结尾的字符串列表，每个字符串都是一个组名称，字符串列表本身是由alloc分配的内存，需要被free释放；字符串不需要释放
 */
extern char** const kdk_conf_list_group(int id);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_config_freeall(char **ptr);

#ifdef __cplusplus
}
#endif

#endif  //LINGMOSDK_BASE_CONFIGURE_H__



 
/**
 * \example lingmosdk-base/src/config/test/test_structlist.c
 * 
 */
/**
  * @}
  */



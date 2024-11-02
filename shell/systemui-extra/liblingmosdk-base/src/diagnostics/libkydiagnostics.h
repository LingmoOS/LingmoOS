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
 * Authors: jishengjie <jishengjie@kylinos.cn>
 *
 */

#ifndef DIAGNOSTICS_KYDIAGNOSTICS_H_
#define DIAGNOSTICS_KYDIAGNOSTICS_H_

/**
 * @file libkydiagnostics.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 上传埋点数据
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (c) 2021
 * 
 * @defgroup liblingmosdk-diagnostics
 * 
 */

typedef struct {
    const char *key;
    const char *value;
} KBuriedPoint;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 上传埋点数据 
 * 
 * @param appName : 包名
 * @param messageType : 消息类型
 * @param data : 埋点数据数组
 * @param length : 数组长度
 * 
 * @retval 0 : 成功
 * @retval -1 : 失败
 */
extern int kdk_buried_point(char *appName , char *messageType , KBuriedPoint *data , int length);

#ifdef __cplusplus
}
#endif

#endif

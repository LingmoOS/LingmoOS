/*
 * liblingmosdk-system's Library
 *
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

#ifndef LIBKYREALTIMEINFO_H
#define LIBKYREALTIMEINFO_H

/**
 * @file libkyrealtimeinfo.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 瞬时信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-realtime 
 * @{
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @brief 获取上传的瞬时网速
 * 
 * @param nc 网卡名称，如eno1
 * @return float 成功返回上传的瞬时网速，失败返回-1.
 */
extern float kdk_real_get_net_speed(const char *nc);

/**
 * @brief 获取CPU温度
 * 
 * 
 * @return double 成功返回CPU温度，失败返回-1.
 */
extern double kdk_real_get_cpu_temperature();

/**
 * @brief 获取磁盘温度
 * 
 * @param name 磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 成功返回磁盘温度，失败返回-1.
 */
extern int kdk_real_get_disk_temperature(const char *name);

/**
 * @brief 获取磁盘转速
 * 
 * @param name 磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 成功返回磁盘转速，失败返回-1.
 */
extern int kdk_real_get_disk_rate(const char *name);

/**
 * @brief 获取下载的瞬时网速信息
 * 
 * @param nc 网卡名称，如eno1
 * @return float 成功返回下载的瞬时网速，失败返回-1.
 */
extern float kdk_real_get_if_speed(const char *nc);

#ifdef __cplusplus
}
#endif

/***********************************
 * 类已弃用 为之前已引用了c++接口的应用保留
 * 新应用获取瞬时信息请使用上面的c接口
************************************/
#ifdef __cplusplus

#define REAL_TIME_EXPORT     __attribute__((visibility("default")))
class REAL_TIME_EXPORT RealTimeInfo
{
public:
    RealTimeInfo();
public:
    float kdk_real_get_io_speed();
    float kdk_real_get_net_speed(const char *nc);
    float kdk_real_get_if_speed(const char *nc);
    float kdk_real_get_cpu_temperature();
    float kdk_real_get_disk_temperature(const char *name);
    int kdk_real_get_disk_rate(const char *name);
};
#endif

#endif // LIBKYREALTIMEINFO_H

/**
 * \example lingmosdk-system/src/realtime/test/kyrealtime-test.c
 * 
 */

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

#ifndef LIBKYBLUETOOTH_H
#define LIBKYBLUETOOTH_H

/**
 * @file libkybluetooth.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 蓝牙信息
 * @version 0.1
 * @date 2023-6-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _kdk_major_class_type{
    MISCELLANEOUS,                  //其它
    COMPUTER,                       //计算机（台式机、笔记本电脑、PDA、组织者等）
    PHONE,                          //电话（蜂窝电话、无绳电话、付费电话、调制解调器等）
    NETWORK_ACCESS_POINT,           //LAN/网络接入点
    AUDIO_VIDEO,                    //音频/视频（耳机、扬声器、立体声、视频显示器、VCR…）
    PERIPHERAL,                     //外围设备（鼠标、操纵杆、键盘…）
    IMAGING,                        //图像处理（打印机、扫描仪、相机、显示器等）
    WEARABLE,                       //可穿戴设备
    TOY,                            //玩具
    HEALTH                          //运行状况设备。 例如，心率监视器。
}kdk_major_class_type;

typedef struct _kdk_major_class
{
    kdk_major_class_type type;          //
    char addr[512];               //设备类
    struct _kdk_major_class *next;
}major_class, *kdk_major_class;

/**
 * @brief 获取蓝牙的设备id
 * 
 * @return int** 蓝牙的设备id
 */
extern int** kdk_bluetooth_get_device_id();

/**
 * @brief 获取蓝牙的制造商
 * 
 * @param num 蓝牙设备的个数，从0开始，0代表1个，以此类推。
 * @return char* 成功返回蓝牙的制造商，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_manufacturer(int num);

/**
 * @brief 获取蓝牙的设备版本
 * 
 * @param num 蓝牙设备的个数，从0开始，0代表1个，以此类推。
 * @return char* 成功返回蓝牙的设备版本，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_dev_version(int num);

/**
 * @brief 获取蓝牙的名称
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的名称，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_name(int id);

/**
 * @brief 获取蓝牙的地址
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的地址，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_address(int id);

/**
 * @brief 获取蓝牙的链接模式
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的链接模式，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_link_mode(int id);

/**
 * @brief 获取蓝牙的链接策略
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的链接策略，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_link_policy(int id);

/**
 * @brief 获取蓝牙的总线
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的总线，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_bus(int id);

/**
 * @brief 获取蓝牙的SCO MTU
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的SCO MTU，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_scomtu(int id);

/**
 * @brief 获取蓝牙的ALC MTU
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的ALC MTU，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_alcmtu(int id);

/**
 * @brief 获取蓝牙的数据包类型
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的数据包类型，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_packettype(int id);

/**
 * @brief 获取蓝牙的功能
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙的功能，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_bluetooth_get_features(int id);

/**
 * @brief 获取蓝牙连接的远程设备的地址（已连接）
 * 
 * @param id 蓝牙的设备id
 * @return char** 成功返回远程设备的地址，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char** kdk_bluetooth_get_device_addr(int id);

/**
 * @brief 获取蓝牙连接的远程设备的名称（已连接）
 * 
 * @param id 蓝牙的设备id
 * @return char** 成功返回远程设备的名称，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char** kdk_bluetooth_get_device_name(int id);

/**
 * @brief 获取蓝牙的连接状态
 * 
 * @param id 蓝牙的设备id
 * @return bool true-已连接，false-未连接
 */
extern bool kdk_bluetooth_get_connection_status(int id);

/**
 * @brief 获取蓝牙地址类型
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙地址类型，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char** kdk_bluetooth_get_addr_type(int id);

/**
 * @brief 获取蓝牙的主要设备类别
 * 
 * @param id 蓝牙的设备id
 * @return char* 成功返回蓝牙地址类型，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern kdk_major_class kdk_bluetooth_get_major_class(int id);

/**
 * @brief 释放由kdk_bluetooth_get_major_class返回的pci总线信息结构体
 * 
 * @param info 由kdk_bluetooth_get_major_class返回的结构体指针
 */
extern void kdk_bluetooth_free_major_class(kdk_major_class info);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_bluetooth_freeall(char **ptr);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kybluetooth-test.c
 * 
 */

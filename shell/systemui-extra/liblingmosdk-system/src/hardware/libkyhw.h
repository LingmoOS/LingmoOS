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

#ifndef KDK_SYSTEM_HW_ASSISTANT_H
#define KDK_SYSTEM_HW_ASSISTANT_H

/**
 * @file libkyhw.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 硬件信息
 * @version 0.1
 * @date 2023-6-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

#define ATTRSIZE 128
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *                      libkyassistant data structures
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

struct HWInfo
{
    char model[ATTRSIZE];       //模块名
    char vendor[ATTRSIZE];      //厂商名
    char device[ATTRSIZE];      //设备名
    char driver[ATTRSIZE];      //设备驱动
    char revision[ATTRSIZE];    //版本
    char busid[ATTRSIZE];       //总线id
    char devicenum[ATTRSIZE];   //设备号，例如鼠标键盘的字符设备号，块设备(光驱)的设备号
    char width[8];              //声卡数据位宽
    char clock[8];              //声卡时钟频率
    struct HWInfo *next;
};

struct power_device
{
    char name[256];                 //设备名
    char native_path[32];           //电源设备的本机路径
    bool power_supply;              //显示 "true" 表示这是一个电源设备，如电池或充电器
    char updated[64];               //上次更新信息的时间戳
    bool has_history;               //显示 "true" 表示设备有历史信息，即充电和放电的历史记录
    bool has_statistics;            //显示 "true" 表示设备有统计信息，例如充电次数、使用时间等。
    bool is_persent;                //显示 "true" 表示电池存在
    bool is_rechargeable;           //显示 "true" 表示电池可充电
    char state[32];                 //电池的当前状态，例如 "charging"、"discharging"、"fully-charged" 等
    char warning_level[32];         //电池电量低于该级别时会触发警告，
    double energy;                  //电池的能量信息，包括当前能量、满电能量等
    double energy_empty;            //电池耗尽时的能量
    double energy_full;             //电池充满时的能量
    double energy_full_design;      //电池的设计容量
    double energy_rate;             //电池的当前充电或放电速率
    double voltage;                 //电池的当前电压
    long time_to_empty;             //电池完全放空（耗尽）所需的时间
    long time_to_full;              //电池完全充满所需的时间
    double percentage;              //电池的电量百分比（电池当前的电量相对于满电容量的百分比）
    double temperature;             //电池的当前温度
    double capacity;                //电池的电量百分比（电池当前充电状态相对于满电的百分比）
    char technology[32];            //电池所使用的技术
    bool online;                    //显示 "true" 表示设备当前在线（连接到电源）
    char icon_name[64];             //显示与设备状态相关联的图标名称
    char model[64];                 //电池型号
    int battery_level;              //电池电量等级
    int type;                       //充电类型
    struct power_device *next;
};

struct Power
{
    char daemon_version[32]; //版本
    bool on_battery;                //系统当前是否依赖电池供电。如果值为'true'，则表示系统正在使用电池供电；如果值为'false'，则表示系统连接到外部电源
    bool lid_is_closed;             //设备的盖子（例如笔记本电脑的盖子）是否关闭。如果值为'true'，则表示盖子已关闭；如果值为'false'，则表示盖子处于打开状态
    bool lid_is_present;            //设备是否具有可关闭的盖子。如果值为'true'，则表示设备具有可关闭的盖子；如果值为'false'，则表示设备没有可关闭的盖子
    char critical_action[32];       //电池电量低于临界水平时系统采取的动作的方法
    struct power_device *devices;
};

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *                      libkyassistant interface function
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

/**
 * @brief 获取硬件信息
 * @param type 3 keyboard; 5 mouse; 15 sound; 23 cdrom; 27 usb
 * @return 硬件信息，具体参考返回结构体
 */
extern struct HWInfo *kdk_hw_get_hwinfo(int type);

/**
 * @brief 释放kdk_hw_get_hwinfo返回的硬件信息结构体
 * 
 * @param 由kdk_hw_get_hwinfo返回的结构体指针
 */
extern void kdk_hw_free_hw(struct HWInfo *list);

/**
 * @brief 获取电源信息
 * 
 * @return 电源信息，具体参考返回结构体
 */
extern struct Power *kdk_hw_get_powerinfo();

/**
 * @brief 释放kdk_hw_get_powerinfo的返回值
 * 
 * @param 由kdk_hw_get_powerinfo返回的结构体指针
 */
extern void kdk_hw_free_power_info(struct Power *info);

#ifdef __cplusplus
}
#endif

#endif // KDK_SYSTEM_HW_ASSISTANT_H

/**
 * \example lingmosdk-system/src/hardware/test/kyhw-test.c
 * 
 */

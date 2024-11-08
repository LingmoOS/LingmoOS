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

#ifndef LIBKYBATTERY_H
#define LIBKYBATTERY_H

/**
 * @file libkybattery.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 电池信息
 * @version 0.1
 * @date 2024-2-20
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

typedef enum _BatteryChargeState
{
    NONE = 0,                   //电池充电状态未知
    ENABLE,                     //电池充电状态为使能状态(充电)
    DISABLE,                    //电池充电状态为停止状态(放电)
    FULL,                       //电池充电状态为已充满状态
    NOT_CHARGING                //电池充电状态为未充电
}BatteryChargeState;


typedef enum _BatteryHealthState
{
    UNKNOWN = 0,                //电池健康状态未知
    GOOD,                       //电池健康状态为正常
    OVERHEAT,                   //电池健康状态为过热
    OVERVOLTAGE,                //电池健康状态为过压
    COLD,                       //电池健康状态为低温
    DEAD,                       //电池健康状态为僵死状态
    UNSPEC_FAILURE,             //电池的健康状态为未指定的故障
    WATCHDOG_TIMER_EXPIRE,      //看门狗定时器到期
    SAFETY_TIMER_EXPIRE,        //安全定时器到期
    OVERCURRENT,                //过电流
    CALIBRATION_REQUIRED,       //需要校准
    WARM,                       //电池的健康状态为温暖；正常状态
    COOL                        //电池的健康状态为凉爽；正常状态
}BatteryHealthState;


typedef enum _BatteryPluggedType
{
    TYPE_NONE = 0,          //连接充电器类型未知
    BATTERY,                //连接的充电器类型为电池
    UPS,                    //连接的充电器类型为不间断电源
    MAINS,                  //连接的充电器类型为交流电源
    USB,                    //连接的充电器类型为USB
    USB_DCP,                //连接的充电器类型为专用于充电的USB端口，提供更大的电流
    USB_CDP,                //通过USB接口连接的可用于下游设备充电的端口
    USB_ACA,                //USB附件充电适配器
    USB_TYPE_C,             //USB Type-C接口
    USB_PD,                 //USB Type-C接口快充
    USB_PD_DRP,             //USB Type-C接口的工作模式，可以充当主机或设备的角色，具有双重作用
    APPLE_BRICK_ID,         //苹果充电方式
    WIRELESS                //连接的充电器类型为无线充电器
}BatteryPluggedType;

/**
 * @brief 获取当前主机电池剩余电量百分比
 *
 * @return float 电池剩余电量百分比
 */
extern float kdk_battery_get_soc();

/**
 * @brief 获取当前主机电池充电状态
 * 
 * @return BatteryChargeState 电池充电状态枚举变量
 */
extern BatteryChargeState kdk_battery_get_charge_state();

/**
 * @brief 获取当前主机电池健康状态
 * 
 * @return BatteryHealthState 电池健康状态枚举变量
 */
extern BatteryHealthState kdk_battery_get_health_state();

/**
 * @brief 获取当前主机连接的充电器类型(未实现)
 * 
 * @return BatteryPluggedType 充电器类型枚举变量
 */
extern BatteryPluggedType kdk_battery_get_plugged_type();

/**
 * @brief 获取当前主机电池电压
 * 
 * @return float 电池电压,单位伏
 */
extern float kdk_battery_get_voltage();

/**
 * @brief 获取当前主机电池技术型号
 * 
 * @return char* 成功返回电池技术型号，失败返回NULL
 */
extern char* kdk_battery_get_technology();

/**
 * @brief 获取当前主机电池温度
 * 
 * @return float 电池温度,单位摄氏度
 */
extern float kdk_battery_get_temperature();

/**
 * @brief 检查当前主机是否支持电池
 * 
 * @return bool 是否支持电池；true-支持，false-不支持
 */
extern bool kdk_battery_is_present();

/**
 * @brief 获取当前主机电池电量等级
 * 
 * @return int 电池电量等级
 */
extern int kdk_battery_get_capacity_level();

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kybios-test.c
 * 
 */
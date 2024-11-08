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

#ifndef LIBKYUSBINFO_H
#define LIBKYUSBINFO_H

/**
 * @file libkyusb.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief usb信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*
    * usb设备信息
    */
typedef struct driver_info
{
    char name[32]; //名称
    char type[2 + 1]; //类型
    char pid[4 + 1]; //设备pid
    char vid[4 + 1]; //设备vid
    char serialNo[32]; //序列号
    char devNode[32]; //路径
} DriverInfo, *pDriverInfo;

/*
    * usb设备信息链表
    */
typedef struct driver_list
{
    struct driver_info *data;
    struct driver_list *next;
} DriverInfoList, *pDriverInfoList;

typedef struct _UsbInfo
{
    unsigned short busNum;  //总线地址
    unsigned short devNum;  //设备地址
    struct _UsbInfo* next;  //链表next指针
}UsbInfo, *pUsbInfo;

typedef struct _UsbDeviceDescriptor
{
    unsigned short lenth;               //描述符的字节数，恒为18
    unsigned short descriptorType;      //描述符类型，恒为0x01
    unsigned short maxPacketSize0;      //端点0的最大数据包大小
    unsigned short numConfigurations;   //可能的配置数量
}UsbDeviceDescriptor, *pUsbDeviceDescriptor;

/**
 * @brief 获取外接usb设备信息
 * 
 * @return pDriverInfoList usb设备信息结构体
 */
extern pDriverInfoList kdk_usb_get_list();

/**
 * @brief 释放kdk_usb_get_list返回的usb信息结构体
 * 
 * @param list kdk_usb_get_list的返回值
 */
extern void kdk_usb_free(pDriverInfoList list);

/**
 * @brief 获取usb设备的总线地址和设备地址
 * 
 * @return pUsbInfo usb设备总线地址和设备地址结构体
 */
extern pUsbInfo kdk_usb_get_bus_info();

/**
 * @brief usb设备是否连接
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return int true 已连接; false 未连接
 */
// extern int kdk_usb_get_found(int busNum, int devAddress);

/**
 * @brief usb硬盘类设备是否挂载
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return int true 已挂载; false 未挂载
 */
extern int kdk_usb_get_mount(int busNum, int devAddress);

/**
 * @brief 获取usb设备的产品信息
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return char* usb设备的产品信息
 */
extern char* kdk_usb_get_productName(int busNum, int devAddress);

/**
 * @brief 获取usb设备厂商信息
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return char* usb设备的厂商信息
 */
extern char* kdk_usb_get_manufacturerName(int busNum, int devAddress);

/**
 * @brief 获取usb设备的版本
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return char* usb设备的版本
 */
extern char* kdk_usb_get_version(int busNum, int devAddress);

/**
 * @brief 获取usb设备的设备类
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return int usb设备的设备类
 */
extern int kdk_usb_get_deviceClass(int busNum, int devAddress);

/**
 * @brief 获取usb设备的设备子类
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return int usb设备的设备子类
 */
extern int kdk_usb_get_deviceSubClass(int busNum, int devAddress);

/**
 * @brief 获取USB设备的协议码
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return int USB设备的协议码
 */
extern int kdk_usb_get_deviceProtocol(int busNum, int devAddress);

/**
 * @brief 获取usb设备的协商连接速度
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return char* usb设备的协商连接速度
 */
extern char* kdk_usb_get_speed(int busNum, int devAddress);

/**
 * @brief 获取usb设备的设备描述符
 * 
 * @param busNum 总线地址
 * @param devAddress 设备地址
 * @return pUsbDeviceDescriptor usb设备的设备描述符
 */
extern pUsbDeviceDescriptor kdk_usb_get_device_descriptor(int busNum, int devAddress);

/**
 * @brief 释放kdk_usb_get_bus_info返回
 * 
 * @param list kdk_usb_get_bus_info返回的指针
 */
extern void kdk_usb_free_usb_info(pUsbInfo list);

/**
 * @brief 释放kdk_usb_get_device_descriptor返回
 * 
 * @param descriptor kdk_usb_get_device_descriptor返回的指针
 */
extern void kdk_usb_free_usb_device_descriptor(pUsbDeviceDescriptor *descriptor);


#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kyusb-test.c
 * 
 */

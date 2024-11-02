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

#ifndef KDK_SYSTEM_HW_NWCARD_H__
#define KDK_SYSTEM_HW_NWCARD_H__
/**
 * @file libkync.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 网卡信息
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * 
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取系统中所有的网卡
 * 
 * @return char** 网卡名称列表，由NULL字符串表示结尾；由alloc生成，需要被kdk_nc_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_nc_get_cardlist();

/**
 * @brief 检测指定网卡是否处于UP状态
 * 
 * @param nc 网卡名称，如eno1
 * @return int Up返回1，Down返回0
 */
extern int kdk_nc_is_up(const char *nc);

/**
 * @brief 获取系统中当前处于 link up 状态的网卡列表
 * 
 * @return char** 网卡名称列表，由NULL字符串表示结尾；由alloc生成，需要被kdk_nc_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_nc_get_upcards();

/**
 * @brief 获取指定网卡的物理MAC地址
 * 
 * @param nc 网卡名称，如eno1
 * @return char* 物理MAC地址，由alloc生成，应当被free；若指定网卡不存在，则返回NULL
 */
extern char* kdk_nc_get_phymac(const char *nc);

/**
 * @brief 获取指定网卡的第一个IPv4地址
 * 
 * @param nc 网卡名称，如eno1
 * @return char* IPv4地址，获取出错或无IP返回NULL
 */
extern char* kdk_nc_get_private_ipv4(const char *nc);

/**
 * @brief 获取指定网卡的所有IPv4地址
 * 
 * @param nc 网卡名称，如eno1
 * @return char** IPv4地址列表，以NULL表示结尾，由alloc生成，需要被kdk_nc_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_nc_get_ipv4(const char* nc);

/**
 * @brief 获取指定网卡的第一个IPv6地址
 * 
 * @param nc 网卡名称，如eno1
 * @return char* IPv6地址，获取出错或无IP返回NULL
 */
extern char* kdk_nc_get_private_ipv6(const char *nc);

/**
 * @brief 获取指定网卡的所有IPv6地址
 *
 * @param nc 网卡名称，如eno1
 * @return char** IPv6地址列表，以NULL表示结尾，由alloc生成，需要被kdk_nc_freeall回收；若获取出错，返回NULL；
 */
extern char **kdk_nc_get_ipv6(const char *nc);

/**
 * @brief 获取指定网卡的有线/无线类型
 * 
 * @param nc 网卡名称，如eno1
 * @return int 0 有线 1 无线
 */
extern int kdk_nc_is_wireless(const char *nc);

/**
 * @brief 获取指定网卡的厂商名称和设备型号
 * 
 * @param nc 网卡名称，如eno1
 * @param vendor 网卡制造厂商，出参，需要提前分配好空足够空间
 * @param porduct 网卡型号，出参，需要提前分配好空足够空间
 * @return int 成功返回 0 失败返回 -1
 */
extern int kdk_nc_get_vendor_and_product(const char *nc, char *vendor, char *product);

/**
 * @brief 获取指定网卡的驱动
 * 
 * @param nc 网卡名称，如eno1
 * @return char * 成功返回网卡驱动，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char *kdk_nc_get_driver(const char *nc);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_nc_freeall(char **ptr);

/**
 * @brief 获取网卡的uuid
 * 
 * @param nc 网卡名称
 * @return char* 网卡的uuid
 */
extern char* kdk_nc_get_uuid(char *nc);

/**
 * @brief 获取网卡的链接状态
 * 
 * @param nc 网卡名称
 * @return int 网卡的链接状态
 */
extern int kdk_nc_get_carrier(char *nc);

/**
 * @brief 获取网卡的工作模式(全双工/半双工)
 * 
 * @param nc 网卡名称
 * @return char* 网卡的工作模式
 */
extern char* kdk_nc_get_duplex(char *nc);

/**
 * @brief 获取指定网卡的广播地址
 * 
 * @param nc 网卡名称，如eno1
 * @param addr ip地址
 * @return char * 成功返回网卡广播地址, 失败返回NULL。返回的字符串需要被 free 释放
 */
extern char *kdk_nc_get_broadAddr(const char *nc, const char *addr);

/**
 * @brief 获取指定网卡的子网掩码
 * 
 * @param nc 网卡名称，如eno1
 * @param addr ip地址
 * @return char * 成功返回子网掩码, 失败返回NULL。返回的字符串需要被 free 释放
 */
extern char *kdk_nc_get_netmask(const char *nc, const char *addr);

/**
 * @brief 获取指定网卡的掩码长度
 * 
 * @param nc 网卡名称，如eno1
 * @param type 1 ipv4，0 ipv6
 * @param addr ip地址
 * @return int 网卡的掩码长度
 */
extern int kdk_nc_get_mask_length(int type, const char *nc, const char *addr);

/**
 * @brief 获取指定网卡的连接类型
 * 
 * @param nc 网卡名称，如eno1
 * @return char * 成功返回网卡的连接类型，如"Ethernet", 失败返回NULL;不需要free。
 */
extern char *kdk_nc_get_conn_type(const char *nc);

/**
 * @brief 获取wifi名称
 * 
 * @param nc 网卡名称，如eno1
 * @return char * 成功返回wifi名称, 返回的字符串需要被 free 释放。
 */
extern char *kdk_nc_get_wifi_name(const char *nc);

/**
 * @brief 获取信号质量(连接 wifi 显示)
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回信号质量, 失败返回-1。
 */
extern int kdk_nc_get_wifi_signal_qual(const char *nc);

/**
 * @brief 获取信号强度(连接 wifi 显示)
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回信号强度, 失败返回-1。
 */
extern int kdk_nc_get_wifi_signal_level(const char *nc);

/**
 * @brief 获取底噪(连接 wifi 显示)
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回底噪, 失败返回-1。
 */
extern int kdk_nc_get_wifi_noise(const char *nc);

/**
 * @brief 获取网卡速率
 * 
 * @param nc 网卡名称，如eno1
 * @return char * 成功返回网卡的速率，如"1000Mb/s", 失败返回NULL;返回的字符串需要被 free 释放。
 */
extern char *kdk_nc_get_speed(const char *nc);

/**
 * @brief 获取指定网卡的接收包数量
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的接收包数量, 失败返回-1。
 */
extern int kdk_nc_get_rx_packets(const char *nc);

/**
 * @brief 获取指定网卡的总计接收
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的总计接收,单位为字节, 失败返回-1。
 */
extern int kdk_nc_get_rx_bytes(const char *nc);

/**
 * @brief 获取指定网卡的接收(错误包)
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的接收(错误包)数量, 失败返回-1。
 */
extern int kdk_nc_get_rx_errors(const char *nc);

/**
 * @brief 获取指定网卡的接收(丢弃包)
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的接收(丢弃包)数量, 失败返回-1。
 */
extern int kdk_nc_get_rx_dropped(const char *nc);

/**
 * @brief 获取指定网卡的接收FIFO
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的接收FIFO数量, 失败返回-1。
 */
extern int kdk_nc_get_rx_fifo_errors(const char *nc);

/**
 * @brief 获取指定网卡的分组帧错误
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的分组帧错误, 失败返回-1。
 */
extern int kdk_nc_get_rx_frame_errors(const char *nc);

/**
 * @brief 获取指定网卡的发送包数量
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的发送包数量, 失败返回-1。
 */
extern int kdk_nc_get_tx_packets(const char *nc);

/**
 * @brief 获取指定网卡的总计发送
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的总计发送, 单位为字节, 失败返回-1。
 */
extern int kdk_nc_get_tx_bytes(const char *nc);

/**
 * @brief 获取指定网卡的发送(错误包)
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的发送(错误包)数量, 失败返回-1。
 */
extern int kdk_nc_get_tx_errors(const char *nc);

/**
 * @brief 获取指定网卡的发送(丢弃包)
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的发送(丢弃包)数量, 失败返回-1。
 */
extern int kdk_nc_get_tx_dropped(const char *nc);

/**
 * @brief 获取指定网卡的发送FIFO
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的发送FIFO数量, 失败返回-1。
 */
extern int kdk_nc_get_tx_fifo_errors(const char *nc);

/**
 * @brief 获取指定网卡的载波损耗
 * 
 * @param nc 网卡名称，如eno1
 * @return int 成功返回网卡的载波损耗, 失败返回-1。
 */
extern int kdk_nc_get_tx_carrier_errors(const char *nc);

/**
 * @brief 获取指定网卡的域
 * 
 * @param nc 网卡名称，如eno1
 * @param addr ip地址
 * @return int 成功返回网卡的域, 失败返回-1。
 */
extern int kdk_nc_get_scope(const char *nc, const char *addr);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kync-test.c
 * 
 */

/**
  * @}
  */

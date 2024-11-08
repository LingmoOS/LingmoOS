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

#ifndef LIBKYNETINFO_H
#define LIBKYNETINFO_H

/**
 * @file libkynetinfo.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 网络信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-net
 * @{
 * 
 */

#define IN 
#define OUT

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct route
{
    char name[32]; //网卡名
    char addr[16]; //网关地址
    struct route *next;
} routeMapList, *prouteMapList;

typedef struct chain_data
{
    char target[8];             //数据包匹配规则后应用的目标动作，目标可以是 ACCEPT（接受）、DROP（丢弃）或其他定义的动作
    char prot[4 + 1];           //数据包使用的协议，可能是 TCP、UDP、ICMP 等
    char opt[4 + 1];            //与规则关联的附加选项
    char source[32];            //数据包的源地址
    char destination[64];       //数据包的目标地址
    char option[128];           //其它选项(匹配条件，动作等)
    struct chain_data *next;
} chainData, *pChainData;

typedef struct chain
{
    char total[16]; //链名
    char policy[32]; //链的规则
    pChainData data;
    struct chain *next;
} Chain, *pChain;

typedef struct _Dhcp4Config
{
    char *broadcast_address;                       // 广播地址
    int dad_wait_time;                             // 指定客户端在接口上等待重复地址检测（DAD）完成的最长时间（以秒为单位）
    int dhcp_lease_time;                           // 动态ip使用时限
    int dhcp_message_type;                         // DHCP报文的类型
    char *dhcp_server_identifier;                  // DNS服务器标识地址
    char *domain_name_servers;                     // 域名服务
    char *expiry;                                  // 分配的IP地址的租约有效期限
    char *ip_address;                              // ip地址
    char *network_number;                          // 网络号
    char *next_server;                             // 下一个网络服务器IP地址
    int requested_broadcast_address;               // 是否请求广播地址
    int requested_domain_name;                     // 是否请求域名
    int requested_domain_name_servers;             // 是否请求域名服务器地址列表
    int requested_domain_search;                   // 是否请求域名搜索列表
    int requested_host_name;                       // 是否请求主机名
    int requested_interface_mtu;                   // 是否请求接口最大传输单元（MTU）
    int requested_ms_classless_static_routes;      // 是否请求无类别静态路由选项
    int requested_netbios_name_servers;            // 是否请求NetBIOS 名称服务器地址
    int requested_netbios_scope;                   // 是否请求NetBIOS 范围
    int requested_ntp_servers;                     // 是否请求 NTP 服务器地址
    int requested_rfc3442_classless_static_routes; // 是否请求 RFC 3442 类无类别静态路由选项
    int requested_root_path;                       // 是否请求根路径信息
    int requested_routers;                         // 是否请求路由器（网关）地址
    int requested_static_routes;                   // 是否请求静态路由信息
    int requested_subnet_mask;                     // 是否请求子网掩码
    int requested_time_offset;                     // 是否请求时间偏移量
    int requested_wpad;                            // 是否请求 Web 代理自动发现（WPAD）
    char *routers;                                 // 路由
    char *server_name;                             // 服务器名称
    char *subnet_mask;                             // 子网掩码
} Dhcp4Config;

typedef struct _Dhcp6Config
{
    int dad_wait_time;                // 指定客户端在接口上等待重复地址检测（DAD）完成的最长时间（以秒为单位）
    char *dhcp6_client_id;            // 客户端标识符
    char *dhcp6_name_servers;         // 域名服务器地址
    char *dhcp6_server_id;            // DHCPv6 服务器标识符
    int requested_dhcp6_client_id;    // 是否请求客户端标识符
    int requested_dhcp_domain_search; // 是否请求域名搜索列表
    int requested_dhp6_name_servers;  // 是否请求域名服务器地址
} Dhcp6Config;

enum NetLinkType
{
    LINK_TYPE_UNKNOWN,
    LINK_TYPE_WIFI,
    LINK_TYPE_ETHERNET,
    LINK_TYPE_WIFI_ETHERNET
};

/**
 * @brief 获取指定网络端口的状态
 * 
 * @param port 端口号
 * @return int 端口状态 TCP_ESTABLISHED:0x1   TCP_SYN_SENT:0x2    TCP_SYN_RECV:0x3    TCP_FIN_WAIT1:0x4    TCP_FIN_WAIT2:0x5
    TCP_TIME_WAIT:0x6    TCP_CLOSE:0x7    TCP_CLOSE_WAIT:0x8    TCP_LAST_ACL:0x9    TCP_LISTEN:0xa    TCP_CLOSING:0xb
 */
extern int kdk_net_get_port_stat(int port);

/**
 * @brief 获取区间端口状态
 * 
 * @param start 开始端口
 * @param end 结束端口
 * @param result 区间端口状态接收缓存区
 * @return int 成功返回0 失败返回-1
 */
extern int kdk_net_get_multiple_port_stat(int start, int end, int *result);

/**
 * @brief 获取网关信息
 *
 * @return prouteMapList 网关信息存储结构体
 */
extern prouteMapList kdk_net_get_route();

/**
 * @brief 释放由kdk_net_get_route返回的网关信息结构体
 * 
 * @param list kdk_net_get_route的返回值
 */
extern void kdk_net_free_route(prouteMapList list);

/**
 * @brief 获取防火墙信息 iptables -L返回
 * 
 * @return pChain 防火墙信息结构体
 */
extern pChain kdk_net_get_iptable_rules();

/**
 * @brief 释放由kdk_net_get_iptable_rules返回的防火墙信息结构体
 *
 * @param list kdk_net_get_iptable_rules的返回值
 */
extern void kdk_net_free_chain(pChain list);

/**
 * @brief 获取子网掩码信息
 * 
 * @param nc 网卡名称，如eno1
 * @param mask 子网掩码
 * 
 */
extern void kdk_net_get_netmask(IN const char *nc, OUT char *mask);

/**
 * @brief 获取进程对应端口
 * 
 * 
 * @return char ** 进程对应端口, 以NULL表示结尾，由alloc生成，需要被kdk_net_freeall回收；若获取出错，返回NULL；
 */
extern char **kdk_net_get_proc_port();

/**
 * @brief 获取所有UP状态的端口号
 * 
 * 
 * @return char ** 所有UP状态的端口号, 以NULL表示结尾，由alloc生成，需要被kdk_net_freeall回收；若获取出错，返回NULL；
 */
extern char **kdk_net_get_up_port();

/**
 * @brief  获取hosts配置
 * 
 * @return char* 成功返回hosts配置，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_net_get_hosts();

/**
 * @brief  获取hosts配置的域名映射
 * 
 * @return char* 成功返回hosts配置的域名映射，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_net_get_hosts_domain();

/**
 * @brief  获取DNS配置文件
 * 
 * @return char** DNS配置文件, 以NULL表示结尾，由alloc生成，需要被kdk_net_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_net_get_resolv_conf();

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_net_freeall(char **ptr);

/**
 * @brief 获取网络连接类型
 * 
 * @return enum NetLinkType 网络连接类型枚举
 */
extern int kdk_net_get_link_type();

/**
 * @brief 检查指定网络连接的连接状态和可用性
 * 
 * @param ip 地址
 * @param port 端口号
 * @return int 是否连接
 */
extern int kdk_net_get_link_status(char *ip, char *port);

/**
 * @brief 获取网络当前已连接的所有网卡名称
 * 
 * @return char** 网卡名列表
 */
extern char** kdk_net_get_link_ncNmae();

/**
 * @brief 获取网络当前使用网卡的网络协议
 * 
 * @return char* 网络当前使用网卡的网络协议
 */
extern char* kdk_net_get_primary_conType();

/**
 * @brief 获取无线设备的工作模式
 * 
 * @param nc 无线网卡名
 * @return char* 无线设备的工作模式
 */
extern char* kdk_net_get_wifi_mode(char *nc);

/**
 * @brief 获取无线设备的工作频率
 * 
 * @param nc 无线网卡名
 * @return char* 无线设备的工作频率
 */
extern char* kdk_net_get_wifi_freq(char *nc);

/**
 * @brief 获取无线设备的频道
 * 
 * @param nc 无线网卡名
 * @return char* 无线设备的通道
 */
extern char* kdk_net_get_wifi_channel(char *nc);

/**
 * @brief 获取无线设备的工作速率
 * 
 * @param nc 无线网卡名
 * @return char* 无线设备的工作速率
 */
extern char* kdk_net_get_wifi_rate(char *nc);

/**
 * @brief 获取无线设备的接收灵敏度的下限
 * 
 * @param nc 无线网卡名
 * @return char* 无线设备的接收灵敏度的下限
 */
extern char* kdk_net_get_wifi_sens(char *nc);

/**
 * @brief 域名转换为ip地址
 * 
 * @param name 域名
 * @return char** ip地址列表
 */
extern char** kdk_net_get_addr_by_name(char *name);

/**
 * @brief ip地址转换为域名
 * 
 * @param ip 网址
 * @return char* 域名
 */
extern char* kdk_net_get_name_by_addr(char *ip);

/**
 * @brief 获取ipv4的dhcp配置(无线连接)
 * 
 * @return Dhcp4Config* 
 */
extern Dhcp4Config *kdk_net_get_ipv4_dhcp_config();

/**
 * @brief 获取ipv6的dhcp配置(无线连接)
 * 
 * @return Dhcp6Config* 
 */
extern Dhcp6Config *kdk_net_get_ipv6_dhcp_config();

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/net/test/kynet-test.c
 * 
 */

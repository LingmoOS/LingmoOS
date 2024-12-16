// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETTYPE_H
#define NETTYPE_H

#include <QMetaType>

namespace ocean {
namespace network {
Q_NAMESPACE

#define NET_WIRED 0x80     // 有线
#define NET_WIRELESS 0x40  // 无线
#define NET_DEVICE 0x01    // 设备
#define NET_CONTROL 0x00   // 控制器
#define NET_MASK_TYPE 0xc0 // 设备类型掩码

class NetType
{
    Q_GADGET
public:
    enum NetItemType {
        Item,                                             // 根节点,0级,虚拟项
        ConnectionItem,                                   // 通用连接项
        WirelessControlItem = NET_WIRELESS | NET_CONTROL, // 无线开关,1级,虚拟项
        WirelessDeviceItem = NET_WIRELESS | NET_DEVICE,   // 无线设备,1级
        WirelessMineItem,                                 // 我的网络,2级,虚拟项
        WirelessOtherItem,                                // 其他网络,2级,虚拟项
        WirelessItem,                                     // 无线项,3级
        WirelessHiddenItem,                               // 无线隐藏项,3级,虚拟项
        WirelessDisabledItem,                             // 无线禁用项,1级,虚拟项
        AirplaneModeTipsItem,                             // 飞行模式提示项,1级
        WiredControlItem = NET_WIRED | NET_CONTROL,       // 有线开关,1级,虚拟项
        WiredDeviceItem = NET_WIRED | NET_DEVICE,         // 有线设备,1级
        WiredItem,                                        // 有线项,2级
        WiredDisabledItem,                                // 有线禁用项,1级,虚拟项
        VPNControlItem,                                   // vpn控制项
        VPNTipsItem,                                      // vpn提示项
        SystemProxyControlItem,                           // 系统代理控制项
        AppProxyControlItem,                              // 应用代理
        DSLControlItem,                                   // dsl控制项
        HotspotControlItem,                               // 热点
        AirplaneControlItem,                              // 飞行模式控制项
        DetailsItem,                                      // 网络详情
        DetailsInfoItem,                                  // 网络详情数据
    };
    Q_ENUM(NetItemType)

    enum NetConnectionStatus {
        CS_UnConnected = 0x01u, // 未连接
        CS_Connecting = 0x02u,  // 正在连接
        CS_Connected = 0x04u,   // 已连接

        CS_AllConnectionStatus = 0x07u //  所有连接状态
    };
    Q_ENUM(NetConnectionStatus)

    // 位定义用于处理优先级，低位用于同级中区分
    enum NetDeviceStatus {
        DS_Unknown = 0x000Fu,           // 未知
        DS_NoCable = 0x001Fu,           // 无网线
        DS_Enabled = 0x003Fu,           // 启用
        DS_Unavailable = 0x00FFu,       // 不可用 优先级同禁用
        DS_Disabled = 0x00FFu,          // 禁用
        DS_ObtainIpFailed = 0x01FFu,    // 获取IP失败
        DS_ConnectFailed = 0x03FFu,     // 连接失败
        DS_Disconnected = 0x07FFu,      // 未连接
        DS_Connected = 0x0FFFu,         // 已连接
        DS_ConnectNoInternet = 0x1FFFu, // 已连接但无网络
        DS_IpConflicted = 0x3FFFu,      // IP冲突
        DS_Connecting = 0x7FFFu,        // 连接中
        DS_ObtainingIP = 0x7FF1u,       // 获取IP
        DS_Authenticating = 0x7FF2u,    // 认证中
    };
    Q_ENUM(NetDeviceStatus)

    enum NetManagerFlag {
        Net_ServiceNM = 0x00000001,         // 使用NM
        Net_MonitorNotify = 0x00000002,     // 发网络通知
        Net_AutoAddConnection = 0x00000004, // 自动添加连接
        Net_UseSecretAgent = 0x00000008,    // 实现密码代理
        Net_CheckPortal = 0x00000010,       // 设置检查认证网页
        Net_tipsLinkEnabled = 0x00000020,   // 可跳转连接

        Net_8021xToConnect = 0x00000100,       // 连接
        Net_8021xToControlCenter = 0x00000200, // 跳转控制中心
        Net_8021xSendNotify = 0x00000400,      // 发通知

        Net_Device = 0x00010000,
        Net_VPN = 0x00020000,
        Net_SysProxy = 0x00040000,
        Net_AppProxy = 0x00080000,
        Net_Hotspot = 0x00100000,
        Net_Airplane = 0x00200000,
        Net_DSL = 0x00400000,
        Net_Details = 0x00800000,

        //
        Net_DockFlags = Net_Device | Net_VPN | Net_SysProxy | Net_UseSecretAgent | Net_CheckPortal | Net_8021xToControlCenter,
        Net_LockFlags = Net_Device | Net_VPN | Net_SysProxy | Net_UseSecretAgent | Net_CheckPortal | Net_8021xSendNotify,
        Net_GreeterFlags = Net_Device | Net_VPN | Net_SysProxy | Net_ServiceNM | Net_MonitorNotify | Net_AutoAddConnection | Net_UseSecretAgent | Net_CheckPortal | Net_8021xSendNotify,
        Net_OceanUIFlags = Net_Device | Net_VPN | Net_SysProxy | Net_AppProxy | Net_Hotspot | Net_Airplane | Net_DSL | Net_Details,
    };

    Q_FLAG(NetManagerFlag);
    Q_DECLARE_FLAGS(NetManagerFlags, NetManagerFlag);

    enum NetItemRoot {
        Net,      // 网络设备
        DSL,      // DSL
        VPN,      // VPN
        SysProxy, // 系统代理
        AppProxy, // 应用代理
        Hotspot,  // 个人热点
        Airplane, // 飞行模式
        NetInfo,  // 网络详情

        Count
    };

    enum ProxyMethod {
        Init = -1,
        None = 0,
        Auto,   // 自动模式,对应字符串”auto”
        Manual, // 手动模式,对应字符串”manual”
    };
    Q_ENUM(ProxyMethod)

    enum ProxyType {
        Http = 0, // http类型
        Https,    // https类型
        Ftp,      // ftp类型
        Socks,    // socks类型
        Socks4,   // sock4类型
        Socks5,   // sock5类型
    };
    Q_ENUM(ProxyType)
};
} // namespace network
} // namespace ocean

Q_DECLARE_METATYPE(ocean::network::NetType::NetItemType)
Q_DECLARE_METATYPE(ocean::network::NetType::NetConnectionStatus)
Q_DECLARE_METATYPE(ocean::network::NetType::NetDeviceStatus)

#endif // NETTYPE_H

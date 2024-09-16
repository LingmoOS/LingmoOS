// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETMANAGER_H
#define NETMANAGER_H

#include <QMap>
#include <QObject>

namespace dde {
namespace network {
class NetItem;
class NetManagerPrivate;

// 业务处理，线程内
class NetManager : public QObject
{
    Q_OBJECT
public:
    explicit NetManager(QObject *parent = nullptr, bool tipsLinkEnabled = true);
    ~NetManager() override;

    void setServiceLoadForNM(bool isNM);        // 使用NM
    void setMonitorNetworkNotify(bool monitor); // 发网络通知
    void setUseSecretAgent(bool enabled);       // 实现密码代理
    void setServerKey(const QString &serverKey);
    // 以上设置需在init之前设置，不支持动态修改
    void init();

    enum ConnectionType {
        Unknown = 0,
        Adsl,
        Bluetooth,
        Bond,
        Bridge,
        Cdma,
        Gsm,
        Infiniband,
        OLPCMesh,
        Pppoe,
        Vlan,
        Vpn,
        Wimax,
        Wired,
        Wireless,
        Team,
        Generic,
        Tun,
        IpTunnel
    };

    enum Network8021XMode {             // 企业网处理方式
        ToControlCenter,                // 跳转控制中心
        SendNotify,                     // 发通知
        ToConnect,                      // 连接
        ToControlCenterUnderConnect,    // 如果设置了此项，则先于配置读取是否直接连接，如果配置中没有，就设置为跳转到控制中心
        SendNotifyUnderConnect          // 如果设置了此项，则先于配置中读取是否直接连接，如果配置中没有，就设置为直接发送消息
    };

    void setNetwork8021XMode(Network8021XMode mode); // 8021X企业网络处理方式，跳转控制中心or发通知
    void setAutoScanInterval(int ms);                // 设置自动扫描无线网间隔，0为不扫描
    void setAutoScanEnabled(bool enabled);           // 设置自动扫描，网络面板关闭时禁用
    void setEnabled(bool enabled);                   // 禁用时不发通知，不请求交互

    const bool isGreeterMode() const;
    NetItem *root() const;
    bool isAirplaneMode() const; // 开启了飞行模式
    NetManager::ConnectionType primaryConnectionType() const;

    enum CmdType {
        EnabledDevice,     // 启用
        DisabledDevice,    // 禁用
        RequestScan,       // 请求刷新
        Disconnect,        // 断开网络
        Connect,           // 连接网络
        UserCancelRequest, // 用户取消输入
        GoToControlCenter, // 跳转到控制中心
        GoToSecurityTools, // 跳转到安全工具
        RequestPassword,   // 请求密码
        CheckInput,        // 检查输入
        InputValid,        // 输入有效(检查输入的返回)
        CloseInput,        // 关闭输入框
        InputError,        // 输入错误
        ToggleExpand       // 切换展开状态
    };
    Q_ENUM(CmdType)

    void exec(CmdType cmd, const QString &id, const QVariantMap &param = QVariantMap()); // 执行操作

    void setWiredEnabled(bool enabled);
    void setWirelessEnabled(bool enabled);
    void setVPNEnabled(bool enabled);
    void setProxyEnabled(bool enabled);

    void updateLanguage(const QString &locale);

    static int StrengthLevel(int strength);
    static QString StrengthLevelString(int level);

public Q_SLOTS:
    void gotoControlCenter();
    void gotoCheckNet();

    bool netCheckAvailable();

Q_SIGNALS:
    void request(CmdType cmd, const QString &id, const QVariantMap &param);
    void airplaneModeChanged(bool enabled);
    // clang-format off
    void networkNotify(const QString &inAppName, int replacesId, const QString &appIcon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expireTimeout);
    // clang-format on
    void languageChange(const QString &locale);
    void toControlCenter();
    void netCheckAvailableChanged(const bool &netCheckAvailable);
    void primaryConnectionTypeChanged(ConnectionType type);

private:
    QScopedPointer<NetManagerPrivate> d_ptrNetManager;
    Q_DECLARE_PRIVATE_D(d_ptrNetManager, NetManager)
    Q_DISABLE_COPY(NetManager)
};
} // namespace network
} // namespace dde

Q_DECLARE_METATYPE(dde::network::NetManager::CmdType)

#endif //  NETMANAGER_H

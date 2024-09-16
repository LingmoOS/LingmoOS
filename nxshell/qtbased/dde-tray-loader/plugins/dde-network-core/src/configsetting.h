// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGSETTING_H
#define CONFIGSETTING_H

#include <QObject>

namespace dde {
namespace network {

class ConfigSetting : public QObject
{
    Q_OBJECT

public:
    static ConfigSetting *instance();
    void alawaysLoadFromNM();               // 始终从NM中加载数据
    bool serviceFromNetworkManager() const; // 来源于NetworkManager的接口，如果是false，则从后端读取数据

    QStringList networkCheckerUrls() const; // 网络检测地址，用于检测网络连通性
    bool enableConnectivity() const;        // 是否开启或者禁用前端检测网络连通性的功能
    bool checkPortal() const;               // 是否检测网络认证信息
    bool supportCertifiedEscape() const;    // 默认是否支持是否回退到未经授权的网络
    bool showUnAuthorizeSwitch() const;     // 是否显示回退到未经授权的网络的开关
    int connectivityCheckInterval() const;  // 网络连通性检测时间间隔
    int wirelessScanInterval() const;       // 无线扫描间隔(ms)
    QString wpaEapAuthen() const;           // 企业网EAP认证方式
    QString wpaEapAuthmethod() const;       // 企业网内部认证方式
    bool networkAirplaneMode() const;       // 控制中心是否显示飞行模式模块
    bool enableAccountNetwork() const;
    bool enableEapInput() const;            // 点击未连接过的企业网是否弹出用户名和密码的输入
    bool dontSetIpIfConflict() const;       // 如果返回true，在手动设置IP之前检测当前IP是否冲突（浙商银行定制）
    bool showBrowserLink() const;           // 显示打开浏览器的文字链接
    QString browserUrl() const;             // 文字链接打开浏览器时默认网址

signals:
    void checkUrlsChanged(const QStringList &);
    void enableConnectivityChanged(bool);
    void checkPortalChanged(bool);
    void connectivityCheckIntervalChanged(int);
    void wirelessScanIntervalChanged(int);
    void wpaEapAuthenChanged(const QString &);
    void wpaEapAuthmethodChanged(const QString &);
    void enableAirplaneModeChanged(bool);
    void supportCertifiedEscapeChanged(bool);
    void showUnAuthorizeSwitchChanged(bool);
    void showBrowserLinkChanged(bool);
    void browserUrlChanged(const QString &);

private:
    explicit ConfigSetting(QObject *parent = nullptr);
    ~ConfigSetting();

private slots:
    void onValueChanged(const QString &key);

private:
    bool m_alwaysFromNM;
    bool m_loadServiceFromNM;
    bool m_enableConnectivity;
    bool m_checkPortal;
    bool m_supportCertifiedEscape;
    bool m_showUnAuthorizeSwitch;
    int m_connectivityCheckInterval;
    int m_wirelessScanInterval;
    QStringList m_networkUrls;
    QString m_wpaEapAuthen;
    QString m_wpaEapAuthmethod;
    bool m_networkAirplaneMode;
    bool m_enableAccountNetwork;
    bool m_enableEapInput;
    bool m_dontSetIpIfConflict;
    bool m_showBrowserLink;
    QString m_browserUrl;
};

} // namespace network
} // namespace dde

#endif // CONFIGSETTING_H

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef KYVPNCONNECTOPERATION_H
#define KYVPNCONNECTOPERATION_H

#include <QSettings>
#include <QDir>

#include "lingmonetworkresourcemanager.h"
#include "lingmoconnectsetting.h"
#include "lingmoconnectoperation.h"
#include "lingmoconnectresource.h"
#include "kyenterpricesettinginfo.h"
#include <NetworkManagerQt/VpnSetting>


#define VPN_SERVERTYPE_L2TP "org.freedesktop.NetworkManager.l2tp"
#define VPN_SERVERTYPE_PPTP "org.freedesktop.NetworkManager.pptp"
#define VPN_SERVERTYPE_OPENVPN "org.freedesktop.NetworkManager.openvpn"
#define VPN_SERVERTYPE_STRONGSWAN "org.freedesktop.NetworkManager.strongswan"

#define KYVPN_VPN_KEY "vpn"

#define KYVPN_GATEWAY_KEY "gateway"
#define KYVPN_REMOTE_KEY "remote"
#define KYVPN_ADDRESS_KEY "address"

#define KYVPN_USER_KEY "user"
#define KYVPN_PASSWD_KEY "password"
#define KYVPN_SECRETS_KEY "secrets"
#define KYVPN_PASSWDFLAGS_KEY "password-flags"

#define KYVPN_REFUSEPAP_KEY "refuse-pap"
#define KYVPN_REFUSECHAP_KEY "refuse-chap"
#define KYVPN_REFUSEEAP_KEY "refuse-eap"
#define KYVPN_REFUSEMSCHAP_KEY "refuse-mschap"
#define KYVPN_REFUSEMSCHAPV2_KEY "refuse-mschapv2"

#define KYVPN_NOVJCOMP_KEY "no-vj-comp"
#define KYVPN_NOACCOMP_KEY "noaccomp"
#define KYVPN_NOBSDCOMP_KEY "nobsdcomp"
#define KYVPN_NODEFLATE_KEY "nodeflate"
#define KYVPN_NOPCOMP_KEY "nopcomp"

#define KYVPN_DOMAIN_KEY "domain"
#define KYVPN_REQUIREMPPE_KEY "require-mppe"
#define KYVPN_MPPESTATEFUL_KEY "mppe-stateful"
#define KYVPN_LCPECHOFAILURE_KEY "lcp-echo-failure"
#define KYVPN_LCPECHOINTERVAL_KEY "lcp-echo-interval"
#define KYVPN_MRU_KEY "mru"
#define KYVPN_MTU_KEY "mtu"

#define KYVPN_REQUIREMPPE128_KEY "require-mppe-128"
#define KYVPN_REQUIREMPPE40_KEY "require-mppe-40"

#define KYVPN_USERCERT_KEY "usercert"
#define KYVPN_USERKEY_KEY "userkey"
#define KYVPN_CERTPASSWD_KEY "cert-pass"
#define KYVPN_CERTPASSWDFLAGS_KEY "cert-pass-flags"
#define KYVPN_PIN_KEY "pin"
#define KYVPN_VIRTUAL_KEY "virtual"
#define KYVPN_ENCAP_KEY "encap"
#define KYVPN_IPCOMP_KEY "ipcomp"
#define KYVPN_PROPOSAL_KEY "proposal"
#define KYVPN_IKE_KEY "ike"
#define KYVPN_ESP_KEY "esp"

//openvpn
#define KYVPN_CONNECTIONTYPE_KEY "connection-type"

#define KYVPN_CA_KEY "ca"
#define KYVPN_CERT_KEY "cert"
#define KYVPN_KEY_KEY "key"

#define KYVPN_USERNAME_KEY "username"
#define KYVPN_STATICKEY_KEY "static-key"
#define KYVPN_STATICKEYDIR_KEY "static-key-direction"
#define KYVPN_LOCALIP_KEY "local-ip"
#define KYVPN_REMOTEIP_KEY "remote-ip"

#define KYVPN_PORT_KEY "port"
#define KYVPN_RENEGSEC_KEY "reneg-seconds"
#define KYVPN_COMPLZO_KEY "comp-lzo"
#define KYVPN_COMPRESS_KEY "compress"
#define COMPLZO_LZODISABLE "no-by-default"
#define COMPLZO_ADAPTIVE "adaptive"
#define COMPRESS_LZO "lzo"
#define COMPRESS_LZ4 "lz4"
#define COMPRESS_LZ4V2 "lz4-v2"

#define KYVPN_PROTOTCP_KEY "proto-tcp"
#define KYVPN_DEVTYPE_KEY "dev-type"
#define DEVTYPE_TUN "tun"
#define DEVTYPE_TAP "tap"
#define KYVPN_DEV_KEY "dev"
#define KYVPN_TUNNELMTU_KEY "tunnel-mtu"
#define KYVPN_FRAGMENTSIZE_KEY "fragment-size"
#define KYVPN_MSSFIX_KEY "mssfix"
#define KYVPN_REMOTERANDOM_KEY "remote-random"
#define KYVPN_TUNIPV6_KEY "tun-ipv6"
#define KYVPN_PING_KEY "ping"
#define KYVPN_PINGEXIT_KEY "ping-exit"
#define KYVPN_PINGRESTART_KEY "ping-restart"
#define KYVPN_FLOAT_KEY "float"
#define KYVPN_MAXROUTES_KEY "max-routes"
#define KYVPN_VERIFYX509NAME_KEY "verify-x509-name"
#define X509NAME_SUBJECT "subject"
#define X509NAME_NAME "name"
#define X509NAME_NAMEPREFIX "name-prefix"

#define KYVPN_REMOTECERTTLS_KEY "remote-cert-tls"
#define KYVPN_NSCERTTYPE_KEY "ns-cert-type"
#define KYVPN_SERVER "server"
#define KYVPN_CLIENT "client"

#define KYVPN_TA_KEY "ta"
#define KYVPN_TLSCRYPT_KEY "tls-crypt"
#define KYVPN_TADIR_KEY "ta-dir"

#define KYVPN_PROXYTYPE_KEY "proxy-type"
#define PROXYTYPE_HTTP "http"
#define PROXYTYPE_SOCKS "socks"
#define KYVPN_PROXYSERVER_KEY "proxy-server"
#define KYVPN_PROXYPORT_KEY "proxy-port"
#define KYVPN_PROXYRETRY_KEY "proxy-retry"
#define KYVPN_HTTPPROXYUSERNAME_KEY "http-proxy-username"
#define KYVPN_HTTPPROXYPASSWD_KEY "http-proxy-password"
#define KYVPN_HTTPPROXYPASSWDFLAGS_KEY "http-proxy-password-flags"

#define KYVPN_KEYSIZE_KEY "keysize"
#define KYVPN_AUTH_KEY "auth"

#define KYVPN_METHOD_KEY "method"

#define KYVPN_YES "yes"
#define KYVPN_NO "no"

//VPN类型
enum KyVpnType{
    KYVPNTYPE_UNKNOWN = -1,
    KYVPNTYPE_L2TP = 0,
    KYVPNTYPE_OPENVPN,
    KYVPNTYPE_PPTP,
    KYVPNTYPE_STRONGSWAN
};

//密码策略
enum KyPasswdPolicy{
    KYPASSWD_FORTHISUSER = 0,
    KYPASSWD_FORALLUSER,
    KYPASSWD_ASKEVERYTIME,
    KYPASSWD_ISNOTREQUIRED = 4
};

//认证方式
enum KyAuthMethod2 {
    KYAUTH2_PAP      = 1u << 0,     //PAP认证
    KYAUTH2_CHAP     = 1u << 1,     //CHAP认证
    KYAUTH2_MSCHAP   = 1u << 2,     //MSCHAP认证
    KYAUTH2_MSCHAPV2 = 1u << 3,     //MSCHAPV2认证
    KYAUTH2_EAP      = 1u << 4,     //EAP认证
    KYAUTH2_ALL      = KYAUTH2_PAP | KYAUTH2_CHAP | KYAUTH2_MSCHAP | KYAUTH2_MSCHAPV2 | KYAUTH2_EAP
};
Q_DECLARE_FLAGS(KyAuthMethods, KyAuthMethod2)

//压缩方式
enum KyCompressMethod {
    KYCOMP_BSD      = 1u << 0,      //允许BSD压缩
    KYCOMP_DEFLATE  = 1u << 1,      //允许Deflate压缩
    KYCOMP_TCP      = 1u << 2,      //允许TCP头压缩
    KYCOMP_PROTO    = 1u << 3,      //使用协议域压缩协商
    KYCOMP_ADDR     = 1u << 4,      //使用地址/控制压缩
    KYCOMP_ALL      = KYCOMP_BSD | KYCOMP_DEFLATE | KYCOMP_TCP | KYCOMP_PROTO | KYCOMP_ADDR
};
Q_DECLARE_FLAGS(KyCompressMethods, KyCompressMethod)

//高级设置
enum KyMPPEMethod {
    KYMPPE_DEFAULT = 0,     //默认MPPE加密
    KYMPPE_REQUIRE128,      //128位加密
    KYMPPE_REQUIRE40        //40位加密
};

enum KyAuthMethod{
    KYAUTH_ERROR = -1,
    KYAUTH_PASSWD = 0,
    KYAUTH_KEY,
    KYAUTH_AGENT,
    KYAUTH_SMARTCARD,
    KYAUTH_EAP,
    KYAUTH_CERTIFICATE,
    KYAUTH_CERTIFICATEANDPASSWD,
    KYAUTH_STATICPASSWD
};

enum KyCompressMethod2{
    KYCOMP2_LZODISABLE = 0,
    KYCOMP2_LZO,
    KYCOMP2_LZ4,
    KYCOMP2_LZ4V2,
    KYCOMP2_LZOADAPTIVE,
    KYCOMP2_AUTO
};

enum KyVirtualDeviceType{
    KYVIRDEVTYPE_TUN = 0,
    KYVIRDEVTYPE_TAP
};

enum KyVpnPingMethod{
    KYVPNPING_EXIT = 0,
    KYVPNPING_RESTART
};

enum KyVpnCheckServerMethod{
    KYCHECKSERVER_NONE = 0,
    KYCHECKSERVER_ENTIRETHEME,
    KYCHECKSERVER_ENTIRENAME,
    KYCHECKSERVER_PRENAME
};

enum KyVpnCertType{
    KYVPNCERT_SERVER = 0,
    KYVPNCERT_CLIENT
};

enum KyVpnTlsMode{
    KYVPNTLS_NONE = 0,
    KYVPNTLS_AUTHENTICATION,
    KYVPNTLS_CRYPT
};

enum KyVpnProxyType{
    KYVPNPROXY_NONE = 0,
    KYVPNPROXY_HTTP,
    KYVPNPROXY_SOCKS
};

enum KyHMACAuthMethod{
    KYHMACAUTH_DEFAULT = 0,
    KYHMACAUTH_NONE,
    KYHMACAUTH_MD4,
    KYHMACAUTH_MD5,
    KYHMACAUTH_SHA1,
    KYHMACAUTH_SHA224,
    KYHMACAUTH_SHA256,
    KYHMACAUTH_SHA384,
    KYHMACAUTH_SHA512,
    KYHMACAUTH_RIPEMD160
};

class KyVpnConfig : public KyConnectSetting
{
public:
    KyVpnType m_vpnType;
    QString m_vpnName;
    QString m_gateway;

    //认证方式
    KyAuthMethod m_authMethod;
    //VPN用户密码信息
    QString m_userName;
    //用户密码
    QString m_userPasswd;
    //密码策略
    KyPasswdPolicy m_passwdPolicy = KYPASSWD_ASKEVERYTIME;
    //NT域
    QString m_ntDomain;

    //CA证书
    QString m_caCertificate;
    //用户证书
    QString m_userCertificate;
    //用户私钥
    QString m_userKey;
    //静态密钥
    QString m_staticKey;
    //用户私有密钥密码
    QString m_privatePasswd;
    //用户私有密码策略
    KyPasswdPolicy m_privatePasswdPolicy = KYPASSWD_ASKEVERYTIME;
    //密钥方向
    QString m_vpnKeyDir;
    //本地IP地址
    QString m_localAddress;
    //远程IP地址
    QString m_remoteAddress;
    //PIN码
    QString m_pinId;

    //MPPE加密开关
    bool m_mppeEnable = false;
    //MPPE加密算法
    KyMPPEMethod m_mppeEncryptMethod = KYMPPE_DEFAULT;
    //使用有状态加密
    bool m_isAllowStatefulEncryption = false;
    //发送PPP回显包
    bool m_sendPppPackage = false;
    //认证方式
    KyAuthMethods m_authMethods = KYAUTH2_ALL;
    //压缩方式
    KyCompressMethods m_compressMethods = KYCOMP_ALL;
    //MRU 最大接收单元
    QString m_mru;
    //MTU 最大传输单元
    QString m_mtu;
    //请求使用内部IP
    bool m_virtual;
    //强制UDP封装
    bool m_encap;
    //使用IP压缩
    bool m_ipcomp;
    //启用自定义密码建议
    bool m_proposal;
    //IKE
    QString m_ike;
    //ESP
    QString m_esp;

    //使用自定义网关端口
    bool m_useAssignPort;
    QString m_assignPort;

    //使用自定义重协商间隔
    bool m_useRenegSeconds;
    QString m_renegSeconds;

    //使用压缩
    bool m_useCompress;
    KyCompressMethod2 m_openvpnCompress;

    //使用TCP连接
    bool m_useTcpLink;

    //设置虚拟设备类型
    bool m_setDevType;
    KyVirtualDeviceType m_devType;

    //设置虚拟设备名称
    bool m_setDevName;
    QString m_devName;

    //使用自定义隧道最大单元传输
    bool m_useTunnelMtu;
    QString m_tunnelMtu;

    //使用自定义UDP分片大小
    bool m_useFragmentSize;
    QString m_fragmentSize;

    //限制TCP最大段尺寸
    bool m_mssfix;

    //随机化远程主机
    bool m_remoteRandom;

    //IPv6 tun link
    bool m_ipv6TunLink;

    //指定Ping周期
    bool m_setPingCycle;
    QString m_pingCycle;

    //指定退出或重启Ping
    bool m_usePingMethod;
    KyVpnPingMethod m_pingMethod;
    QString m_pingMethodTime;

    //接受来自任何地址（Float）已通过身份验证的数据包
    bool m_float;

    //指定路由上限
    bool m_setMaxRoute;
    QString m_maxRoute;

    //服务器证书检验
    KyVpnCheckServerMethod m_checkServerCa;

    //主题匹配
    QString m_verifyName;

    //验证对等（服务器）证书用法签名
    bool m_useRemoteCertTls;
    KyVpnCertType m_remoteCertType;

    //验证对等（服务器）证书nsCertType签名
    bool m_useNsCertTls;
    KyVpnCertType m_nsCertType;

    //其他TLS身份验证
    //模式
    KyVpnTlsMode m_vpnTlsMode;
    //密钥文件
    QString m_vpnKeyFile;
    //密钥方向
    QString m_vpnTlsTaDir;

    //代理
    //代理类型
    KyVpnProxyType m_vpnProxyType;
    //服务器地址
    QString m_vpnProxyServer;
    //端口
    QString m_vpnProxyPort;
    //出错时无限重试
    bool m_vpnProxyRetry;
    //代理用户名
    QString m_vpnProxyName;
    //代理用户名
    QString m_vpnProxyPasswd;
    //代理密码策略
    KyPasswdPolicy m_vpnProxyPasswdPolicy = KYPASSWD_ASKEVERYTIME;

    //安全
    //使用自定义密钥大小
    bool m_useKeysize;
    QString m_keySize;
    //HMAC认证
    KyHMACAuthMethod m_hmacAuthMethod;

    //IPv4Setting
    QStringList m_ipv4DnsSearch;
    QString m_ipv4DhcpClientId;

    //IPv6Setting
    QStringList m_ipv6DnsSearch;
};



class KyVpnConnectOperation : public KyConnectOperation
{
    Q_OBJECT
public:
    explicit KyVpnConnectOperation(QObject *parent = nullptr);

public:
    void createVpnConnect(KyVpnConfig &vpnSettings);
    void setVpnConfig(QString connectUuid, KyVpnConfig &vpnConfig);
    KyVpnConfig getVpnConfig(QString connectUuid);

    void activateVpnConnection(const QString connectUuid);
    void deactivateVpnConnection(const QString activeConnectName, const QString &activeConnectUuid);
    void deleteVpnConnect(const QString &connectUuid);

private:
    KyHMACAuthMethod getHmacAuthMethod(const QString method);
    KyAuthMethod getAuthMethod(const QString method);

    void getConnectionSetting(QString connectUuid, KyVpnConfig &vpnConfig);
    void setConnectionSetting(NetworkManager::ConnectionSettings::Ptr connectionSettings, KyVpnConfig &vpnConfig);

    void getL2tpConfig(KyVpnConfig &vpnConfig);
    void setL2tpConfig(KyVpnConfig &vpnConfig);
    void getPptpConfig(KyVpnConfig &vpnConfig);
    void setPptpConfig(KyVpnConfig &vpnConfig);
    void getStrongswanConfig(KyVpnConfig &vpnConfig);
    void setStrongswanConfig(KyVpnConfig &vpnConfig);
    void getOpenvpnConfig(KyVpnConfig &vpnConfig);
    void setOpenvpnConfig(KyVpnConfig &vpnConfig);

    void getUsrPasswdAndPolicy(KyVpnConfig &vpnConfig);
    void setUsrPasswdAndPolicy(KyVpnConfig &vpnConfig);

    void getCertPasswdAndPolicy(KyVpnConfig &vpnConfig);
    void setCertPasswdAndPolicy(KyVpnConfig &vpnConfig);

    NMVariantMapMap setIpConfig(NetworkManager::ConnectionSettings::Ptr connectionSettings, KyVpnConfig &vpnConfig);

private:
    QMap<QString, KyAuthMethod> m_authMethodMap = {
        {"password"         , KYAUTH_PASSWD                 },
        {"key"              , KYAUTH_KEY                    },
        {"agent"            , KYAUTH_AGENT                  },
        {"smartcard"        , KYAUTH_SMARTCARD              },
        {"eap"              , KYAUTH_EAP                    },
        {"tls"              , KYAUTH_CERTIFICATE            },
        {"password-tls"     , KYAUTH_CERTIFICATEANDPASSWD   },
        {"static-password"  , KYAUTH_STATICPASSWD           }
    };

    QMap<QString, KyHMACAuthMethod> m_hmacMap = {
        {"none"         , KYHMACAUTH_NONE       },
        {"RSA-MD4"      , KYHMACAUTH_MD4        },
        {"MD5"          , KYHMACAUTH_MD5        },
        {"SHA1"         , KYHMACAUTH_SHA1       },
        {"SHA224"       , KYHMACAUTH_SHA224     },
        {"SHA256"       , KYHMACAUTH_SHA256     },
        {"SHA384"       , KYHMACAUTH_SHA384     },
        {"SHA512"       , KYHMACAUTH_SHA512     },
        {"RIPEMD160"    , KYHMACAUTH_RIPEMD160  }
    };

    NMStringMap m_vpnData;
    NMStringMap m_vpnSecrets;
};

#endif // KYVPNCONNECTOPERATION_H

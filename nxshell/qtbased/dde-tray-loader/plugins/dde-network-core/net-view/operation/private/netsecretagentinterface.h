// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETSECRETAGENTINTERFACE_H
#define NETSECRETAGENTINTERFACE_H

#include <QVariant>

namespace dde {
namespace network {

// GetSecrets超时时间，参考NM中nm-secret-agent.c中函数nm_secret_agent_get_secrets里的设置
#define GET_SECRETS_TIMEOUT 120000

class NetSecretAgentInterface
{
public:
    typedef std::function<void(const QString &dev, const QString &ssid, const QVariantMap &secrets)> PasswordCallbackFunc;

    explicit NetSecretAgentInterface(PasswordCallbackFunc fun);
    virtual ~NetSecretAgentInterface();

    void requestPassword(const QString &dev, const QString &id, const QVariantMap &param);
    void cancelRequestPassword(const QString &dev, const QString &id);
    virtual bool hasTask() = 0;
    virtual void inputPassword(const QString &key, const QVariantMap &password, bool input) = 0;

private:
    PasswordCallbackFunc m_passwordCallbackFunc;
};

} // namespace network
} // namespace dde
#endif // NETSECRETAGENTINTERFACE_H

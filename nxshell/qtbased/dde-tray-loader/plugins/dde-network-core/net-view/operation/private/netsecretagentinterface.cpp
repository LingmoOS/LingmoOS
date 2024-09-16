// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netsecretagentinterface.h"

namespace dde {
namespace network {

NetSecretAgentInterface::NetSecretAgentInterface(NetSecretAgentInterface::PasswordCallbackFunc fun)
    : m_passwordCallbackFunc(fun)
{
}

NetSecretAgentInterface::~NetSecretAgentInterface() = default;

void NetSecretAgentInterface::requestPassword(const QString &dev, const QString &id, const QVariantMap &param)
{
    m_passwordCallbackFunc(dev, id, param);
}

void NetSecretAgentInterface::cancelRequestPassword(const QString &dev, const QString &id)
{
    m_passwordCallbackFunc(dev, id, {});
}

} // namespace network
} // namespace dde

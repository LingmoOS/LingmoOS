// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef PROXYCONTROLLER_INTER_H
#define PROXYCONTROLLER_INTER_H

#include "proxycontroller.h"

class ProxyChains;

namespace dde {
namespace network {

class ProxyController_Inter : public ProxyController
{
    Q_OBJECT

public:
    explicit ProxyController_Inter(QObject *parent);
    void setAppProxy(const AppProxyConfig &config);  // 设置应用代理数据
    void setAppProxyEnabled(bool enabled);
    bool appProxyEnabled() const;

private:
    ProxyChains *m_chainsInter;
};

}
}

#endif // PROXYCONTROLLER_INTER_H

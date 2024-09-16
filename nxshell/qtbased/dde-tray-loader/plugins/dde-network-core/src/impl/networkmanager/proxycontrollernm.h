// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef PROXYCONTROLLERNM_H
#define PROXYCONTROLLERNM_H

#include "proxycontroller.h"

namespace dde {
namespace network {

class ProxyController_NM : public ProxyController
{
    Q_OBJECT

public:
    explicit ProxyController_NM(QObject *parent);
    void setAppProxy(const AppProxyConfig &config);  // 设置应用代理数据
    void setAppProxyEnabled(bool enabled);
    bool appProxyEnabled() const;

private:
    void initMember();
    void initConnection();

private slots:
    void onPropertiesChanged(const QString &interface, const QVariantMap &values);
};

}
}

#endif // PROXYCONTROLLERNM_H

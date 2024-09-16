// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPPROXYMODULE_H
#define APPPROXYMODULE_H
#include "interface/pagemodule.h"

#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DTextEdit;
DWIDGET_END_NAMESPACE
namespace DCC_NAMESPACE {
class SettingsGroup;
class LineEditWidget;
class ButtonTuple;
class SwitchWidget;
class ComboxWidget;
}

namespace dde {
namespace network {
class ControllItems;
enum class ProxyMethod;
}
}

class AppProxyModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit AppProxyModule(QObject *parent = nullptr);
    virtual void deactive() override;

Q_SIGNALS:
    void resetData();

private Q_SLOTS:
    void onCheckValue();

private:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    bool isIPV4(const QString &ipv4);

private:
    DCC_NAMESPACE::ComboxWidget *m_proxyType;
    DCC_NAMESPACE::LineEditWidget *m_addr;
    DCC_NAMESPACE::LineEditWidget *m_port;
    DCC_NAMESPACE::LineEditWidget *m_username;
    DCC_NAMESPACE::LineEditWidget *m_password;
    DCC_NAMESPACE::ButtonTuple *m_btns;
};

#endif // APPPROXYMODULE_H

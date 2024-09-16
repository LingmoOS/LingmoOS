// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSPROXYMODULE_H
#define SYSPROXYMODULE_H
#include "interface/pagemodule.h"

#include <QPointer>

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

class SysProxyModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit SysProxyModule(QObject *parent = nullptr);

    virtual void active() override;
    virtual void deactive() override;

private Q_SLOTS:
    void initManualView(QWidget *w);
    void applySettings();
    void uiMethodChanged(dde::network::ProxyMethod uiMethod);
    void resetData(dde::network::ProxyMethod uiMethod);
    void checkConf();

private:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<DCC_NAMESPACE::ModuleObject *> m_modules;
    const QStringList m_ProxyMethodList;

    QPointer<DCC_NAMESPACE::SwitchWidget> m_proxySwitch;
    QPointer<DCC_NAMESPACE::ComboxWidget> m_proxyTypeBox;

    QPointer<DCC_NAMESPACE::LineEditWidget> m_autoUrl;

    QPointer<DCC_NAMESPACE::LineEditWidget> m_httpAddr;
    QPointer<DCC_NAMESPACE::LineEditWidget> m_httpPort;
    QPointer<DCC_NAMESPACE::LineEditWidget> m_httpsAddr;
    QPointer<DCC_NAMESPACE::LineEditWidget> m_httpsPort;
    QPointer<DCC_NAMESPACE::LineEditWidget> m_ftpAddr;
    QPointer<DCC_NAMESPACE::LineEditWidget> m_ftpPort;
    QPointer<DCC_NAMESPACE::LineEditWidget> m_socksAddr;
    QPointer<DCC_NAMESPACE::LineEditWidget> m_socksPort;
    QPointer<DTK_WIDGET_NAMESPACE::DTextEdit> m_ignoreList;
    QPointer<DCC_NAMESPACE::ButtonTuple> m_buttonTuple;

    dde::network::ProxyMethod m_uiMethod; // ui修改的ProxyMethod
    int m_lastCombIndex = 0;
};

#endif // SYSPROXYMODULE_H

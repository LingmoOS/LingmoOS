// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNMODULE_H
#define VPNMODULE_H
#include "interface/pagemodule.h"

#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DListView;
DWIDGET_END_NAMESPACE

namespace dde {
namespace network {
class ControllItems;
}
}

class VPNModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit VPNModule(QObject *parent = nullptr);

private Q_SLOTS:
    void initVPNList(DTK_WIDGET_NAMESPACE::DListView *vpnView);
    void editConnection(dde::network::ControllItems *item, QWidget *parent);
    void importVPN();
    void changeVpnId();

private:
    QList<DCC_NAMESPACE::ModuleObject *> m_modules;
    QString m_newConnectionPath;
    QString m_editingConnUuid;
};

#endif // VPNMODULE_H

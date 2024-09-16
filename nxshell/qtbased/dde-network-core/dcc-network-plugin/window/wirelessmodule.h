// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIRELESSMODULE_H
#define WIRELESSMODULE_H

#include "interface/pagemodule.h"

#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DListView;
DWIDGET_END_NAMESPACE

namespace dde {
namespace network {
class AccessPoints;
class WirelessDevice;
}
}

class WirelessModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit WirelessModule(dde::network::WirelessDevice *dev, QObject *parent = nullptr);

public Q_SLOTS:
    virtual void active() override;

private Q_SLOTS:
    void onNetworkAdapterChanged(bool checked);
    void onApWidgetEditRequested(dde::network::AccessPoints *ap, QWidget *parent);
    void initWirelessList(DTK_WIDGET_NAMESPACE::DListView *lvAP);
    void onNameChanged(const QString &name);

private:
    dde::network::WirelessDevice *m_device;
};

#endif // WIRELESSMODULE_H

// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNL2TPSETTINGS_H
#define VPNL2TPSETTINGS_H

#include "settings/abstractsettings.h"

#include <networkmanagerqt/connectionsettings.h>

class VpnL2tpSettings : public AbstractSettings
{
    Q_OBJECT

public:
    explicit VpnL2tpSettings(NetworkManager::ConnectionSettings::Ptr connSettings, QWidget *parent = Q_NULLPTR);
    virtual ~VpnL2tpSettings() Q_DECL_OVERRIDE;

protected:
    void initSections() Q_DECL_OVERRIDE;
    bool setInterfaceName() Q_DECL_OVERRIDE { return false; }
};

#endif

// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNPPTPSETTINGS_H
#define VPNPPTPSETTINGS_H

#include "settings/abstractsettings.h"

#include <networkmanagerqt/connectionsettings.h>

class VpnPPTPSettings : public AbstractSettings
{
    Q_OBJECT

public:
    explicit VpnPPTPSettings(NetworkManager::ConnectionSettings::Ptr connSettings, QWidget *parent = Q_NULLPTR);
    virtual ~VpnPPTPSettings() Q_DECL_OVERRIDE;

protected:
    void initSections() Q_DECL_OVERRIDE;
    bool setInterfaceName() Q_DECL_OVERRIDE { return false; }
};

#endif /* VPNPPTPSETTINGS_H */

// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNOPENVPNSETTINGS_H
#define VPNOPENVPNSETTINGS_H

#include "settings/abstractsettings.h"

#include <QVBoxLayout>

#include <networkmanagerqt/connectionsettings.h>

class VpnOpenVPNSettings : public AbstractSettings
{
    Q_OBJECT

public:
    explicit VpnOpenVPNSettings(NetworkManager::ConnectionSettings::Ptr connSettings, QWidget *parent = nullptr);
    virtual ~VpnOpenVPNSettings() Q_DECL_OVERRIDE;

protected:
    void initSections() Q_DECL_OVERRIDE;
    bool setInterfaceName() Q_DECL_OVERRIDE { return false; }
};

#endif /* VPNOPENVPNSETTINGS_H */

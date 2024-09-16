// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNOPENCONNECTSETTINGS_H
#define VPNOPENCONNECTSETTINGS_H

#include "settings/abstractsettings.h"

#include <networkmanagerqt/connectionsettings.h>

class VpnOpenConnectSettings : public AbstractSettings
{
    Q_OBJECT

public:
    explicit VpnOpenConnectSettings(NetworkManager::ConnectionSettings::Ptr connSettings, QWidget *parent = nullptr);
    virtual ~VpnOpenConnectSettings() Q_DECL_OVERRIDE;

protected:
    void initSections() Q_DECL_OVERRIDE;
    bool setInterfaceName() Q_DECL_OVERRIDE { return false; }
};

#endif /* VPNOPENCONNECTSETTINGS_H */

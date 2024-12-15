// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNVPNCSETTINGS_H
#define VPNVPNCSETTINGS_H

#include "settings/abstractsettings.h"

#include <networkmanagerqt/connectionsettings.h>

#include <QVBoxLayout>

class VpnVPNCSettings : public AbstractSettings
{
    Q_OBJECT

public:
    explicit VpnVPNCSettings(NetworkManager::ConnectionSettings::Ptr connSettings, QWidget *parent = nullptr);
    virtual ~VpnVPNCSettings() Q_DECL_OVERRIDE;

protected:
    void initSections() Q_DECL_OVERRIDE;
    bool setInterfaceName() Q_DECL_OVERRIDE { return false; }
};

#endif /* VPNVPNCSETTINGS_H */

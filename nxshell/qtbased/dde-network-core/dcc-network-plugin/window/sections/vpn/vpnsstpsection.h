// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNSSTPSECTION_H
#define VPNSSTPSECTION_H

#include "vpnsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace dcc {
namespace network {
class FileChooseWidget;
}
}

namespace DCC_NAMESPACE {
class SwitchWidget;
}

class VpnSSTPSection : public VpnSection
{
    Q_OBJECT

public:
    explicit VpnSSTPSection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnSSTPSection() Q_DECL_OVERRIDE;

    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initUI();

private:
    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;

    dcc::network::FileChooseWidget *m_caFile;
    DCC_NAMESPACE::SwitchWidget *m_ignoreCAWarnings;
    DCC_NAMESPACE::SwitchWidget *m_useTLSExt;
};

#endif /* VPNSSTPSECTION_H */

/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Douglas Kosovic <doug@uq.edu.au>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_L2TP_PPP_WIDGET_H
#define PLASMA_NM_L2TP_PPP_WIDGET_H

#include <QDialog>

#include <NetworkManagerQt/VpnSetting>

namespace Ui
{
class L2tpPppWidget;
}

class L2tpPPPWidget : public QDialog
{
    Q_OBJECT
public:
    explicit L2tpPPPWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr, bool need_peer_eap = false);
    ~L2tpPPPWidget() override;

    NMStringMap setting() const;

private:
    void loadConfig(const NetworkManager::VpnSetting::Ptr &setting);
    Ui::L2tpPppWidget *const m_ui;
    const bool m_need_peer_eap;
};

#endif // PLASMA_NM_L2TP_PPP_WIDGET_H

/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Douglas Kosovic <doug@uq.edu.au>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_L2TP_IPSEC_WIDGET_H
#define PLASMA_NM_L2TP_IPSEC_WIDGET_H

#include <QDialog>

#include <NetworkManagerQt/VpnSetting>

namespace Ui
{
class L2tpIpsecWidget;
}

class L2tpIpsecWidget : public QDialog
{
    Q_OBJECT

    enum AuthType { PSK = 0, TLS };
    enum IpsecDaemonType { NoIpsecDaemon, Libreswan, Strongswan, Openswan, UnknownIpsecDaemon };

public:
    explicit L2tpIpsecWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr);
    ~L2tpIpsecWidget() override;

    NMStringMap setting() const;
    NMStringMap secrets() const;

    static bool hasIpsecDaemon();

private Q_SLOTS:
    void updateStartDirUrl(const QUrl &);
    void setDefaultIkelifetime(bool isChecked);
    void setDefaultSalifetime(bool isChecked);
    void resizeStackedWidget(int currentIndex);

private:
    void loadConfig(const NetworkManager::VpnSetting::Ptr &setting);
    Ui::L2tpIpsecWidget *const m_ui;
    static IpsecDaemonType m_ipsecDaemonType;
};

#endif // PLASMA_NM_L2TP_IPSEC_WIDGET_H

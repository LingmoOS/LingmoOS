/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_VPNC_ADVANCED_WIDGET_H
#define PLASMA_NM_VPNC_ADVANCED_WIDGET_H

#include <QDialog>

#include <NetworkManagerQt/VpnSetting>

namespace Ui
{
class VpncAdvancedWidget;
}

class VpncAdvancedWidget : public QDialog
{
    Q_OBJECT
public:
    explicit VpncAdvancedWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr);
    ~VpncAdvancedWidget() override;

    NMStringMap setting() const;
    void loadConfig(const NetworkManager::VpnSetting::Ptr &setting);

private:
    Ui::VpncAdvancedWidget *const m_ui;
};

#endif // PLASMA_NM_VPNC_ADVANCED_WIDGET_H

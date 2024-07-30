/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_IPV6_WIDGET_H
#define PLASMA_NM_IPV6_WIDGET_H

#include "plasmanm_editor_export.h"

#include <NetworkManagerQt/Ipv6Setting>
#include <QWidget>

#include "ipv6routeswidget.h"
#include "settingwidget.h"

namespace Ui
{
class IPv6Widget;
}

class PLASMANM_EDITOR_EXPORT IPv6Widget : public SettingWidget
{
    Q_OBJECT
public:
    enum MethodIndex { Automatic = 0, AutomaticOnlyIP, AutomaticOnlyDHCP, LinkLocal, Manual, Ignored, Disabled };

    explicit IPv6Widget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~IPv6Widget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void slotModeComboChanged(int index);
    void slotRoutesDialog();

    void slotDnsServers();
    void slotDnsDomains();

    void slotAddIPAddress();
    void slotRemoveIPAddress();

    void selectionChanged(const QItemSelection &selected);
    void tableViewItemChanged(QStandardItem *item);

private:
    Ui::IPv6Widget *const m_ui;
    NetworkManager::Ipv6Setting m_tmpIpv6Setting;

    class Private;
    Private *const d;
};

#endif // PLASMA_NM_IPV4_WIDGET_H

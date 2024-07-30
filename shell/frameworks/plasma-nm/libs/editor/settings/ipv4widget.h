/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_IPV4_WIDGET_H
#define PLASMA_NM_IPV4_WIDGET_H

#include "plasmanm_editor_export.h"

#include <NetworkManagerQt/Ipv4Setting>
#include <QWidget>

#include "ipv4routeswidget.h"
#include "settingwidget.h"

namespace Ui
{
class IPv4Widget;
}

class PLASMANM_EDITOR_EXPORT IPv4Widget : public SettingWidget
{
    Q_OBJECT
public:
    enum MethodIndex { Automatic = 0, AutomaticOnlyIP, LinkLocal, Manual, Shared, Disabled };

    explicit IPv4Widget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~IPv4Widget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void slotModeComboChanged(int index);
    void slotRoutesDialog();
    void slotAdvancedDialog();

    void slotDnsServers();
    void slotDnsDomains();

    void slotAddIPAddress();
    void slotRemoveIPAddress();

    void selectionChanged(const QItemSelection &selected);
    void tableViewItemChanged(QStandardItem *item);

private:
    Ui::IPv4Widget *const m_ui;
    NetworkManager::Ipv4Setting m_tmpIpv4Setting;

    class Private;
    Private *const d;
};

#endif // PLASMA_NM_IPV4_WIDGET_H

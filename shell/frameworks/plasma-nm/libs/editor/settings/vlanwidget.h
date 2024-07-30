/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_VLAN_WIDGET_H
#define PLASMA_NM_VLAN_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include "settingwidget.h"

namespace Ui
{
class VlanWidget;
}

class PLASMANM_EDITOR_EXPORT VlanWidget : public SettingWidget
{
    Q_OBJECT

public:
    explicit VlanWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~VlanWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private:
    void fillConnections();
    Ui::VlanWidget *const m_ui;
};

#endif // PLASMA_NM_VLAN_WIDGET_H

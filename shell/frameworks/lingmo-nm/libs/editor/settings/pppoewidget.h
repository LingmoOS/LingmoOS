/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_PPPOE_WIDGET_H
#define LINGMO_NM_PPPOE_WIDGET_H

#include "lingmonm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/Setting>

#include "settingwidget.h"

namespace Ui
{
class PppoeWidget;
}

class LINGMONM_EDITOR_EXPORT PppoeWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit PppoeWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~PppoeWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private:
    Ui::PppoeWidget *const m_ui;
};

#endif // LINGMO_NM_PPPOE_WIDGET_H

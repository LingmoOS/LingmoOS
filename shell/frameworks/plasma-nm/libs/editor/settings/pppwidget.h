/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_PPP_WIDGET_H
#define PLASMA_NM_PPP_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/Setting>

#include "settingwidget.h"

namespace Ui
{
class PPPWidget;
}

class PLASMANM_EDITOR_EXPORT PPPWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit PPPWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~PPPWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

private:
    Ui::PPPWidget *const m_ui;
};

#endif // PLASMA_NM_PPP_WIDGET_H

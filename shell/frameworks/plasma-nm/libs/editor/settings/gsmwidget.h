/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_GSM_WIDGET_H
#define PLASMA_NM_GSM_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/Setting>

#include "settingwidget.h"

namespace Ui
{
class GsmWidget;
}

class PLASMANM_EDITOR_EXPORT GsmWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit GsmWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~GsmWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private:
    Ui::GsmWidget *const m_ui;
};

#endif // PLASMA_NM_GSM_WIDGET_H

/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_BT_WIDGET_H
#define PLASMA_NM_BT_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/BluetoothSetting>

#include "settingwidget.h"

namespace Ui
{
class BtWidget;
}

class PLASMANM_EDITOR_EXPORT BtWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit BtWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~BtWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private:
    Ui::BtWidget *const m_ui;
};

#endif // PLASMA_NM_BT_WIDGET_H

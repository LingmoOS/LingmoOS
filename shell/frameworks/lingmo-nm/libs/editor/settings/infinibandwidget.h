/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_INFINIBAND_WIDGET_H
#define LINGMO_NM_INFINIBAND_WIDGET_H

#include "lingmonm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/Setting>

#include "settingwidget.h"

namespace Ui
{
class InfinibandWidget;
}

class LINGMONM_EDITOR_EXPORT InfinibandWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit InfinibandWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(), QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~InfinibandWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private:
    Ui::InfinibandWidget *const m_ui;
};

#endif // LINGMO_NM_INFI_WIDGET_H

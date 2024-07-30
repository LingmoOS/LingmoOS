/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_WIRED_CONNECTION_WIDGET_H
#define PLASMA_NM_WIRED_CONNECTION_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include "settingwidget.h"

namespace Ui
{
class WiredConnectionWidget;
}

class PLASMANM_EDITOR_EXPORT WiredConnectionWidget : public SettingWidget
{
    Q_OBJECT

public:
    enum LinkNegotiation {
        Ignore = 0,
        Automatic,
        Manual,
    };

    enum Duplex {
        Half = 0,
        Full,
    };

    explicit WiredConnectionWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~WiredConnectionWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void generateRandomClonedMac();

private:
    Ui::WiredConnectionWidget *const m_widget;
};

#endif // PLASMA_NM_WIRED_CONNECTION_WIDGET_H

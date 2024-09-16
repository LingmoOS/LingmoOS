// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>
#include <QWidget>

namespace dwclock {
class ClockPanel;
class SettingsView;
class TimezoneModel;
class ViewManager : public QObject
{
    Q_OBJECT
public:
    explicit ViewManager (QObject *parent = nullptr);
    virtual ~ViewManager() override;
    ClockPanel *clockPanel(int roundedCornerRadius) const;
    SettingsView *settingsView() const;
    TimezoneModel *model() const;
    void updateModel(const QStringList &timezones);

private:
    TimezoneModel *m_model = nullptr;
    mutable ClockPanel *m_clockPanel = nullptr;
    mutable QPointer<SettingsView> m_settingsView = nullptr;
};
}

// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainview.h"

#include "clockpanel.h"
#include "settingsview.h"
#include "timezonemodel.h"

namespace dwclock {
ViewManager::ViewManager(QObject *parent)
    : QObject (parent)
    , m_model(new TimezoneModel())
{
}

ViewManager::~ViewManager()
{
    m_model->deleteLater();
    m_model = nullptr;
}

ClockPanel *ViewManager::clockPanel(int roundedCornerRadius) const
{
    if (!m_clockPanel) {
        m_clockPanel = new ClockPanel();
        m_clockPanel->setRoundedCornerRadius(roundedCornerRadius);
        m_clockPanel->view()->setModel(m_model);
    }
    return m_clockPanel;
}

SettingsView *ViewManager::settingsView() const
{
    if (!m_settingsView) {
        TimezoneModel *m = new TimezoneModel();
        m->appendItems(m_model->timezones());
        m_settingsView = new SettingsView(m, m_clockPanel);
        m->setParent(m_settingsView);
    }
    return m_settingsView;
}

TimezoneModel *ViewManager::model() const
{
    return m_model;
}

void ViewManager::updateModel(const QStringList &timezones)
{
    m_model->updateModel(timezones);
}
}

// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "quickpanelwidget.h"
#include "airplanemodecontroller.h"

QuickPanelWidget::QuickPanelWidget(QWidget *parent)
    : SignalQuickPanel(parent)
{
    connect(this, &QuickPanelWidget::clicked, AMC_PTR, &AirplaneModeController::toggle);
    connect(AMC_PTR, &AirplaneModeController::enabledChanged, this, [this] (bool enabled) {
        setWidgetState(enabled ? WidgetState::WS_ACTIVE : WidgetState::WS_NORMAL);
    });
    setWidgetState(AMC_PTR->isEnabled() ? WidgetState::WS_ACTIVE : WidgetState::WS_NORMAL);
    connect(AMC_PTR, &AirplaneModeController::prepareForSleep, this, [this](bool sleep) {
        if (!sleep) {
            setWidgetState(AMC_PTR->isEnabled() ? WidgetState::WS_ACTIVE : WidgetState::WS_NORMAL);
        }
    });
}

QuickPanelWidget::~QuickPanelWidget()
{

}
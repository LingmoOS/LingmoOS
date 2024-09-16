// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "quickpanelwidget.h"
#include "dndmodecontroller.h"

QuickPanelWidget::QuickPanelWidget(QWidget* parent)
    : SignalQuickPanel(parent)
{
    connect(this, &QuickPanelWidget::clicked, &DndModeController::ref(), &DndModeController::toggle);
    connect(&DndModeController::ref(), &DndModeController::dndModeChanged, this, [this](bool enabled) {
        setWidgetState(enabled ? WidgetState::WS_ACTIVE : WidgetState::WS_NORMAL);
    });
    setWidgetState(DndModeController::ref().isDndModeEnabled() ? WidgetState::WS_ACTIVE : WidgetState::WS_NORMAL);
}

QuickPanelWidget::~QuickPanelWidget()
{
}
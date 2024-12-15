// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "quickpanelwidget.h"
#include "clipboardcontroller.h"

QuickPanelWidget::QuickPanelWidget(QWidget* parent)
    : SignalQuickPanel(parent)
{
    connect(this, &QuickPanelWidget::clicked, [this] {
        ClipboardController::ref().toggle();
        Q_EMIT requestHideApplet();
    });
}

QuickPanelWidget::~QuickPanelWidget()
{
}
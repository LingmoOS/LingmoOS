// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include "instancepanel.h"

DWIDGET_USE_NAMESPACE

WIDGETS_FRAME_BEGIN_NAMESPACE
class DisplayModePanelCell : public InstancePanelCell {
    Q_OBJECT
public:
    explicit DisplayModePanelCell(Instance *instance, QWidget *parent = nullptr);

    virtual void setView() override;
};

class DisplayModePanel : public InstancePanel {
    Q_OBJECT
public:
    explicit DisplayModePanel (WidgetManager *manager, QWidget *parent = nullptr);
    void init();

    virtual DisplayModePanelCell *createWidget(Instance *instance) override;

Q_SIGNALS:
    void editClicked();
};
WIDGETS_FRAME_END_NAMESPACE

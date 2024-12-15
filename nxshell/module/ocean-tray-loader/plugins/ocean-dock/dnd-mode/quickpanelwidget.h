// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUICKPANELWIDGET_H
#define QUICKPANELWIDGET_H

#include "singlequickpanel.h"

class QuickPanelWidget : public SignalQuickPanel
{
    Q_OBJECT
public:
    QuickPanelWidget(QWidget *parent = nullptr);
    ~QuickPanelWidget();

private:

};

#endif
// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "quickpanel.h"

QuickPanel::QuickPanel(QWidget *parent)
    : SignalQuickPanel(parent)
{
    setIcon(QIcon::fromTheme(":/icons/icon/keyboard-symbolic.svg"));
}

QuickPanel::~QuickPanel()
{

}
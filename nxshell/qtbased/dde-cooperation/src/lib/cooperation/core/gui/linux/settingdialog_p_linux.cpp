// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/dialogs/settingdialog.h"
#include "gui/dialogs/settingdialog_p.h"

#include <DTitlebar>

DWIDGET_USE_NAMESPACE
using namespace cooperation_core;

void SettingDialogPrivate::initTitleBar()
{
    DTitlebar *titleBar = new DTitlebar(q);
    titleBar->setMenuVisible(false);
    titleBar->setIcon(QIcon::fromTheme("dde-cooperation"));

    mainLayout->insertWidget(0, titleBar);
}

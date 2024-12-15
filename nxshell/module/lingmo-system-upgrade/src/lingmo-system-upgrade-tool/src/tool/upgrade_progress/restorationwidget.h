// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DFontSizeManager>

#include "../../core/dbusworker.h"
#include "upgradeprogresswidget.h"

class RestorationWidget: public UpgradeProgressWidget
{
    Q_OBJECT
public:
    RestorationWidget(QWidget *parent);

private:
    DBusWorker *m_dbusWorker;

    void initUI();
    void initConnections();
};

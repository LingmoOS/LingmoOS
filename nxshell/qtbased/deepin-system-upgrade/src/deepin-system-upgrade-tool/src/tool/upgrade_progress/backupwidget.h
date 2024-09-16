// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "../../core/dbusworker.h"
#include "upgradeprogresswidget.h"

class BackupWidget : public UpgradeProgressWidget
{
    Q_OBJECT
public:
    BackupWidget(QWidget *parent);

private:
    DBusWorker *m_dbusWorker;

    void initUI();
    void initConnections();
};

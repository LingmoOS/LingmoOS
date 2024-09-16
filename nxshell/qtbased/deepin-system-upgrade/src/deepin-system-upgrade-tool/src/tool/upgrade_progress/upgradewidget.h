// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "../../core/dbusworker.h"
#include "upgradeprogresswidget.h"

class UpgradeWidget: public UpgradeProgressWidget
{
    Q_OBJECT
public:
    UpgradeWidget(QWidget *parent);

Q_SIGNALS:
    void start(const QString isoPath);

private:
    DBusWorker *m_dbusWorker;

    void initUI();
    void initConnections();
};

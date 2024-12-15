// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "../../widgets/basecontainerwidget.h"
#include "../../core/constants.h"
#include "../../core/dbusworker.h"
#include "upgradecheckresultwidget.h"
#include "upgradecheckprogresswidget.h"

DWIDGET_USE_NAMESPACE

class UpgradeCheckWidget: public BaseContainerWidget
{
    Q_OBJECT
public:
    explicit UpgradeCheckWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void nextStage();
    void previousStage();

private:
    DBusWorker                  *m_dbusworker;
    QStackedLayout              *m_stackedlayout;
    UpgradeCheckProgressWidget  *m_upgradecheckprogresswidget;
    UpgradeCheckResultWidget    *m_upgradecheckresultwidget;

    void initUI();
    void initConnections();

private Q_SLOTS:
    void runUpgradeChecks();
    void jumpToResult();
    void onCancelButtonClicked();
};

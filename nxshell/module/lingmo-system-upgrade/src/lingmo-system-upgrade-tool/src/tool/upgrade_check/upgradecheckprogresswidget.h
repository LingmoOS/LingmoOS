// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DWidget>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../../core/constants.h"
#include "../../core/dbusworker.h"
#include "../../widgets/iconlabel.h"
#include "../../widgets/progresscirclewidget.h"

DWIDGET_USE_NAMESPACE

/**
 * @brief The progress view of upgrade check. After all checks are done, it is required to jump to UpgradeCheckResultWidget.
 *
 */
class UpgradeCheckProgressWidget: public DWidget
{
    Q_OBJECT
public:
    explicit UpgradeCheckProgressWidget(DWidget *parent = nullptr);

Q_SIGNALS:
    void AllCheckDone();

private:
    DLabel                  *m_titleLabel;
    IconLabel               *m_conditionsCheckLabel;
    IconLabel               *m_softwareChangesLabel;
    DBusWorker              *m_dbusWorker;
    QVBoxLayout             *m_mainVBoxLayout;
    QVBoxLayout             *m_checklistVBoxLayout;
    QHBoxLayout             *m_panelHBoxLayout;
    ProgressCircleWidget    *m_checkProgress;

    void initUI();
    void initConnections();

Q_SIGNALS:
    void CheckDone();

private Q_SLOTS:
    void onCheckProgressUpdate(qint64 progress);
    void onCheckDone();
};

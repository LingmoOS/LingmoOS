// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DLabel>
#include <DWidget>

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../../core/constants.h"
#include "../../core/dbusworker.h"
#include "../../widgets/progresscirclewidget.h"

DWIDGET_USE_NAMESPACE

class SoftwareCheckProgressWidget: public DWidget
{
    Q_OBJECT
public:
    explicit SoftwareCheckProgressWidget(DWidget *parent = nullptr);

Q_SIGNALS:
    void AllCheckDone();

private:
    DLabel                  *m_titleLabel;
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
    void onCheckProgressUpdate(int progress);
    void onCheckDone();
};

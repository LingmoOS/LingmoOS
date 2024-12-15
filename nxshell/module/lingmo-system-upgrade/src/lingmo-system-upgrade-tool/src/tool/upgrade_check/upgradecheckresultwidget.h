// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DLabel>
#include <DPushButton>
#include <DSuggestButton>
#include <DButtonBox>

#include <QStackedLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "../../core/dbusworker.h"
#include "../../core/constants.h"
#include "../../widgets/baselabel.h"
#include "../../widgets/iconlabel.h"
#include "conditionchecklistwidget.h"

DWIDGET_USE_NAMESPACE

class UpgradeCheckResultWidget: public QWidget
{
    Q_OBJECT
public:
    explicit UpgradeCheckResultWidget(QWidget *parent = nullptr);
    bool passed() const {
        return m_isCheckPassed;
    }

Q_SIGNALS:
    void nextStage();
    void previousStage();
    void updateUI();
    void updateCompleted();

private Q_SLOTS:
    void onUpdateUI();
    void updateIcons();

private:
    bool                        m_isCheckPassed = true;
    DBusWorker                  *m_dbusWorker;
    QVBoxLayout                 *m_mainVerticalLayout;
    QStackedLayout              *m_resultContentLayout;
    IconLabel                   *m_titleIconLabel;
    DLabel                      *m_titleTextLabel;
    ConditionChecklistWidget    *m_conditionCheckWidget;

    void initUI();
    void initConnections();
};

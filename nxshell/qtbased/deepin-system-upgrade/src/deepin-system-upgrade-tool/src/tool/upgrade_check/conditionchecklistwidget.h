// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QVBoxLayout>
#include <QLabel>

#include "../../widgets/statusicon.h"
#include "../../widgets/iconlabel.h"
#include "../../widgets/storageresultwidget.h"
#include "../../widgets/checkresultwidget.h"
#include "../../widgets/baselabel.h"


DWIDGET_USE_NAMESPACE

class ConditionChecklistWidget: public QWidget
{
    Q_OBJECT

public:
    ConditionChecklistWidget(QWidget *parent);
    bool passed() {
        return m_isPassed;
    }

Q_SIGNALS:
    void updateCompleted();

private:
    bool m_isPassed = false;

    QVBoxLayout         *m_mainLayout;
    DLabel              *m_warningTextLabel;
    CheckResultWidget   *m_versionResultWidget;
#ifdef SHOW_ACTIVATION
    CheckResultWidget   *m_activationResultWidget;
#endif
    CheckResultWidget   *m_cpuResultWidget;
    StorageResultWidget *m_storageResultWidget;

    void initUI();
    void initConnections();

public Q_SLOTS:
    void updateUI();
};

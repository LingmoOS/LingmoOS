// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QStackedLayout>

#include "../../core/dbusworker.h"
#include "../../widgets/basecontainerwidget.h"
#include "../../widgets/errormessagewidget.h"
#include "upgradewidget.h"
#include "backupwidget.h"
#include "restorationwidget.h"

class SystemUpgradeWidget: public BaseContainerWidget
{
    Q_OBJECT
public:
    SystemUpgradeWidget(QWidget *parent);

Q_SIGNALS:
    void start(const QString isoPath);

private Q_SLOTS:
    void jumpToBackupWidget();
    void setupErrorWidget(QString errorTitle, QString errorLog);
    void jumpErrorWidget();

private:
    DBusWorker          *m_dbusWorker;
    QStackedLayout      *m_stackedLayout;
    UpgradeWidget       *m_upgradeWidget;
    BackupWidget        *m_backupWidget;
    RestorationWidget   *m_restorationWidget;
    ErrorMessageWidget  *m_errorMessageWidget;

    void initUI();
    void initConnections();

};

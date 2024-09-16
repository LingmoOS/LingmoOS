// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QStackedLayout>
#include <QWidget>

#include "../../core/dbusworker.h"
#include "../../widgets/basecontainerwidget.h"
#include "../../widgets/errormessagewidget.h"
#include "migrateprogresswidget.h"
#include "migratefinishedwidget.h"

class MigrateWidget: public BaseContainerWidget {
    Q_OBJECT
public:
    MigrateWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void start();

private:
    DBusWorker              *m_dbusWorker;
    ErrorMessageWidget      *m_errorWidget;
    MigrateProgressWidget   *m_progressWidget;
    MigrateFinishedWidget   *m_finishedWidget;
    QStackedLayout          *m_stackedLayout;

    void initUI();
    void initConnections();
};

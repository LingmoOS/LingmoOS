// SPDX-FileCopyrightText: 2022- 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDesktopServices>

#include "../mainwindow.h"
#include "migratewidget.h"

MigrateWidget::MigrateWidget(QWidget *parent)
    :   BaseContainerWidget (parent)
    ,   m_dbusWorker(DBusWorker::getInstance())
    ,   m_errorWidget(new ErrorMessageWidget(this))
    ,   m_progressWidget(new MigrateProgressWidget(this))
    ,   m_finishedWidget(new MigrateFinishedWidget(this))
    ,   m_stackedLayout(new QStackedLayout(this))
{
    initUI();
    initConnections();
}

void MigrateWidget::initUI()
{
    m_spacerItem->changeSize(0, 0);
    m_cancelButton->setText(tr("Exit"));
    m_cancelButton->setFixedSize(120, 36);
    m_suggestButton->setText(tr("Submit Feedback"));
    m_suggestButton->setFixedSize(120, 36);
    m_stackedLayout->addWidget(m_progressWidget);
    m_stackedLayout->addWidget(m_errorWidget);
    m_stackedLayout->addWidget(m_finishedWidget);
    m_contentLayout->addLayout(m_stackedLayout);
}

void MigrateWidget::initConnections()
{
    connect(this, &MigrateWidget::start, m_progressWidget, &MigrateProgressWidget::start);
    connect(m_dbusWorker, &DBusWorker::MigrateError, [this] (int status, const QString msg) {
        if (status == 1)
        {
            emit m_progressWidget->networkError();
        }
        else if (status > 1)
        {
            m_errorWidget->setTitle(tr("App migration failed"));
            m_errorWidget->setErrorLog(msg);
            m_stackedLayout->setCurrentWidget(m_errorWidget);
            m_cancelButton->setVisible(true);
            m_suggestButton->setVisible(true);
        }
    });
    connect(m_progressWidget, &MigrateProgressWidget::done, [this] {
        m_stackedLayout->setCurrentWidget(m_finishedWidget);
    });
    connect(m_cancelButton, &DPushButton::clicked, [this] {
        MainWindow::getInstance()->close();
    });
    connect(m_suggestButton, &DSuggestButton::clicked, [this] {
        // Feedback
        QDesktopServices::openUrl(QUrl("https://bbs.deepin.org/"));
    });
}

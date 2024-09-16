// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QDesktopWidget>

#include "../mainwindow.h"
#include "systemupgradewidget.h"
#include "../../widgets/alertdialog.h"

SystemUpgradeWidget::SystemUpgradeWidget(QWidget *parent)
    : BaseContainerWidget(parent, 2)
    , m_dbusWorker(DBusWorker::getInstance(this))
    , m_stackedLayout(new QStackedLayout(this))
    , m_upgradeWidget(new UpgradeWidget(this))
    , m_backupWidget(new BackupWidget(this))
    , m_restorationWidget(new RestorationWidget(this))
    , m_errorMessageWidget(new ErrorMessageWidget(this))
{
    initUI();
    initConnections();
}

void SystemUpgradeWidget::initUI()
{
    m_spacerItem->changeSize(0, 0);
    m_stackedLayout->addWidget(m_upgradeWidget);
    m_stackedLayout->addWidget(m_backupWidget);
    m_stackedLayout->addWidget(m_restorationWidget);
    m_stackedLayout->addWidget(m_errorMessageWidget);
    m_contentLayout->addLayout(m_stackedLayout);
}

void SystemUpgradeWidget::initConnections()
{
    connect(this, &SystemUpgradeWidget::start, m_upgradeWidget, &UpgradeWidget::start);
    connect(m_cancelButton, &QPushButton::clicked, [] {
        MainWindow::getInstance()->close();
    });
    connect(m_suggestButton, &QPushButton::clicked, [this] {
        if (m_errorMessageWidget->title() == tr("Backup failed")) {
            // Backup again
            jumpToBackupWidget();
        } else {
            // Feedback when upgrade failed
            QDesktopServices::openUrl(QUrl("https://bbs.deepin.org/"));
        }
    });
    connect(m_upgradeWidget, &UpgradeWidget::done, this, &SystemUpgradeWidget::jumpToBackupWidget);
    connect(m_backupWidget, &BackupWidget::done, [this] {
        AlertDialog dlg(this);
        if (dlg.execWarningDialog(nullptr, tr("After clicking restart, the system will be upgraded, please save the current running application data before confirming"), tr("Restart now")) == 0)
        {
            // Start upgrade when backup is done.
            emit m_dbusWorker->StartUpgrade();
        } else {
            this->deleteLater();
            exit(0);
        }
    });
    // Jump to ErrorWidget for DBus errors
    connect(m_dbusWorker, &DBusWorker::error, [this] (QString errorTitle, QString errorLog) {
        if (DBusWorker::getInstance()->getUpgradeStage() == UpgradeStage::INIT)
        {
            // No restoration procedure for initial stage. (Just reused by main program)
            setupErrorWidget(errorTitle, errorLog);
            jumpErrorWidget();
        }
        else if (errorTitle == tr("Restoration failed"))
        {
            // Restoration failed, show error log directly.
            setupErrorWidget(errorTitle, errorLog);
            jumpErrorWidget();
        }
        else
        {
            m_stackedLayout->setCurrentWidget(m_restorationWidget);
            emit m_restorationWidget->start();
            // Prepare error view in order to jump directly once system restoration is done.
            setupErrorWidget(errorTitle, errorLog);
        }
    });
    connect(m_restorationWidget, &RestorationWidget::done, this, &SystemUpgradeWidget::jumpErrorWidget);
}

void SystemUpgradeWidget::jumpToBackupWidget()
{
    m_cancelButton->setVisible(false);
    m_suggestButton->setVisible(false);
    m_stackedLayout->setCurrentWidget(m_backupWidget);
    emit m_backupWidget->start();
}

void SystemUpgradeWidget::setupErrorWidget(QString errorTitle, QString errorLog)
{
    if (errorTitle == tr("Backup failed"))
    {
        m_cancelButton->setFixedSize(120, 36);
        m_cancelButton->setText(tr("Exit"));

        m_suggestButton->setFixedSize(120, 36);
        m_suggestButton->setText(tr("Try Again"));
    }
    else
    {
        m_cancelButton->setFixedSize(120, 36);
        m_cancelButton->setText(tr("Exit"));

        m_suggestButton->setFixedSize(120, 36);
        m_suggestButton->setText(tr("Submit Feedback"));
    }
    m_errorMessageWidget->setTitle(errorTitle);
    m_errorMessageWidget->setErrorLog(errorLog);
}

void SystemUpgradeWidget::jumpErrorWidget()
{
    m_cancelButton->setVisible(true);
    m_suggestButton->setVisible(true);
    m_stackedLayout->setCurrentWidget(m_errorMessageWidget);
}

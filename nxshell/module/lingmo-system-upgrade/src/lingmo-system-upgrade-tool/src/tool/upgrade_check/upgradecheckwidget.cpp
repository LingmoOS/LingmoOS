// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <string>
#include <iostream>

#include "../application.h"
#include "upgradecheckwidget.h"
#include "../mainwindow.h"
#include "../../widgets/alertdialog.h"

#define CHECK_RESULT_CONTENT_BOTTOM_SPACE_H 100
#define BUTTON_W 120
#define BUTTON_H 36

UpgradeCheckWidget::UpgradeCheckWidget(QWidget *parent)
    : BaseContainerWidget(parent, 0)
    , m_dbusworker(DBusWorker::getInstance())
    , m_stackedlayout(new QStackedLayout(this))
    , m_upgradecheckprogresswidget(new UpgradeCheckProgressWidget(this))
    , m_upgradecheckresultwidget(new UpgradeCheckResultWidget(this))
{
    initUI();
    initConnections();
}

void UpgradeCheckWidget::initUI()
{
    m_threeDotsWidget->setVisible(true);
    m_stackedlayout->addWidget(m_upgradecheckprogresswidget);
    m_stackedlayout->addWidget(m_upgradecheckresultwidget);
    m_contentLayout->addLayout(m_stackedlayout);
    m_stackedlayout->setCurrentIndex(0);
}

void UpgradeCheckWidget::initConnections()
{
    connect(m_upgradecheckprogresswidget, &UpgradeCheckProgressWidget::AllCheckDone, m_upgradecheckresultwidget, &UpgradeCheckResultWidget::updateUI);
    connect(m_upgradecheckresultwidget, SIGNAL(updateCompleted()), this, SLOT(jumpToResult()));
    connect(m_dbusworker, &DBusWorker::StartUpgradeCheck, this, &UpgradeCheckWidget::runUpgradeChecks);
    connect(m_upgradecheckresultwidget, SIGNAL(previousStage()), this, SIGNAL(previousStage()));
    connect(m_upgradecheckresultwidget, SIGNAL(previousStage()), this, SLOT(runUpgradeChecks()));
    connect(m_cancelButton, &QPushButton::clicked, this, &UpgradeCheckWidget::onCancelButtonClicked);
    connect(m_midButton, &QPushButton::clicked, m_dbusworker, &DBusWorker::StartUpgradeCheck);
    connect(m_suggestButton, &DSuggestButton::clicked, this, &UpgradeCheckWidget::nextStage);
}

void UpgradeCheckWidget::onCancelButtonClicked()
{
    std::cout << "UpgradeCheck Cancel Button clicked" << std::endl;
    if (m_stackedlayout->currentIndex() == 0) {
        AlertDialog dlg(this, tr("Do you want to exit the pre-upgrade checking?"));
        if (dlg.exec() == DDialog::Accepted)
        {
            Q_EMIT previousStage();
            Q_EMIT m_dbusworker->cancelUpgradeCheck();
        }
    }
    else
    {
        MainWindow::getInstance()->close();
    }
}

void UpgradeCheckWidget::runUpgradeChecks()
{
    // Change the size of placeholders under the contents, avoid distortions of widgets.
    m_spacerItem->changeSize(0, 20);
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setEnabled(true);
    m_cancelButton->setVisible(true);
    m_cancelButton->setFixedWidth(250);
    m_midButton->setVisible(false);
    m_suggestButton->setVisible(false);
    m_stackedlayout->setCurrentIndex(0);
}

void UpgradeCheckWidget::jumpToResult()
{
    m_cancelButton->setText(tr("Exit"));
    m_cancelButton->setEnabled(true);
    m_cancelButton->setVisible(true);
    m_cancelButton->setFixedSize(BUTTON_W, BUTTON_H);

    m_midButton->setText(tr("Check Again"));
    m_midButton->setVisible(true);
    m_midButton->setFixedSize(BUTTON_W, BUTTON_H);

    m_suggestButton->setText(tr("Next"));
    m_suggestButton->setVisible(true);
    m_suggestButton->setFixedSize(BUTTON_W, BUTTON_H);
    m_suggestButton->setEnabled(m_upgradecheckresultwidget->passed());

    // Adjust the placeholder size below.
    m_spacerItem->changeSize(0, CHECK_RESULT_CONTENT_BOTTOM_SPACE_H, QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_stackedlayout->setCurrentIndex(1);
}

// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "migrateprogresswidget.h"
#include "../../core/dbusworker.h"

MigrateProgressWidget::MigrateProgressWidget(QWidget *parent)
    : ProgressWidget(parent)
    , m_reconnectButton(new DSuggestButton(this))
{
    initUI();
    initConnections();
}

void MigrateProgressWidget::initUI()
{
    m_titleLabel->setFont(QFont("SourceHanSansSC", 15, QFont::DemiBold));
    m_tipLabel->setFont(QFont("SourceHanSansSC", 10.5, QFont::Normal));
    m_tipLabel->setForegroundRole(DPalette::TextTitle);
    m_etaLabel->setFont(QFont("SourceHanSansSC", 9, QFont::Normal));
    m_etaLabel->setVisible(false);
    m_iconSpacerItem->changeSize(0, 0);
    m_tipSpacerItem->changeSize(0, 46);
    m_iconLabel->setPixmap(QIcon(":/icons/system_upgrade.svg").pixmap(128, 128));
    m_titleLabel->setText(tr("Migrating apps"));
    m_tipLabel->setText(tr("Migrating apps, please wait patiently"));
    m_reconnectButton->setText(tr("Reconnect"));
    m_reconnectButton->setVisible(false);
    m_mainLayout->addWidget(m_reconnectButton, 0, Qt::AlignCenter);
}

void MigrateProgressWidget::initConnections()
{
    connect(this, &MigrateProgressWidget::start, this, &MigrateProgressWidget::onStart);
    connect(this, &MigrateProgressWidget::start, [] {
        DBusWorker::getInstance()->MigratePackages();
    });
    connect(DBusWorker::getInstance(), &DBusWorker::AppCheckProgressUpdate, [this] (int progress) {
        emit ProgressWidget::updateProgress(progress);
    });
    connect(DBusWorker::getInstance(), &DBusWorker::MigrateDone, [this] () {
       emit done();
    });
    connect(m_reconnectButton, &DSuggestButton::clicked, [this] {
        DBusWorker::getInstance()->MigratePackages();
        m_iconLabel->setPixmap(QIcon(":/icons/system_upgrade.svg").pixmap(128, 128));
        m_titleLabel->setText(tr("Migrating apps"));
        m_tipLabel->setText(tr("Migrating apps, please wait patiently"));
        m_reconnectButton->setText(tr("Reconnect"));
        m_reconnectButton->setVisible(false);
    });
    connect(this, &MigrateProgressWidget::networkError, [this] {
        m_iconLabel->setPixmap(QIcon(":/icons/network_error.svg").pixmap(128, 128));
        m_titleLabel->setText(tr("Network error. App migration has been interrupted."));
        m_tipLabel->setText(tr("Network connection failed. Please check your network."));
        m_reconnectButton->setVisible(true);
    });
}

// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DFontSizeManager>

#include <QPixmap>
#include <QColor>
#include <QFont>

#include "backupwidget.h"

BackupWidget::BackupWidget(QWidget *parent)
    : UpgradeProgressWidget(parent)
    , m_dbusWorker(DBusWorker::getInstance(this))
{
    initUI();
    initConnections();
}

void BackupWidget::initUI()
{
    m_iconLabel->setPixmap(QIcon(":/icons/system_backup.svg").pixmap(ICON_SIZE, ICON_SIZE));
    m_titleLabel->setText(tr("Backing up…"));
    m_tipLabel->setText(tr("Backing up the current system, please wait patiently"));
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T4, QFont::Bold);
    DFontSizeManager::instance()->bind(m_tipLabel, DFontSizeManager::T6, QFont::Medium);
}

void BackupWidget::initConnections()
{
    connect(this, &BackupWidget::start, m_dbusWorker, &DBusWorker::StartBackup);
    connect(m_dbusWorker, &DBusWorker::BackupProgressUpdate, this, [this] (int progress) {
        emit ProgressWidget::updateProgress(progress);
        if (progress == 100)
        {
            emit done();
        }
    });
    connect(m_dbusWorker, &DBusWorker::BackupProgressUpdate, this, &BackupWidget::updateDefaultEtaText);
}

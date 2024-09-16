// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QPixmap>
#include <QColor>
#include <QFont>

#include "restorationwidget.h"

RestorationWidget::RestorationWidget(QWidget *parent)
    : UpgradeProgressWidget(parent)
    , m_dbusWorker(DBusWorker::getInstance(this))
{
    initUI();
    initConnections();
}

void RestorationWidget::initUI()
{
    m_iconLabel->setPixmap(QIcon(":/icons/system_backup.svg").pixmap(ICON_SIZE, ICON_SIZE));
    m_titleLabel->setText(tr("Restoring…"));
    m_tipLabel->setText(tr("Upgrade failed. Your system is being restored to the previous version."));
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T4, QFont::Bold);
    DFontSizeManager::instance()->bind(m_tipLabel, DFontSizeManager::T6, QFont::Medium);
}

void RestorationWidget::initConnections()
{
    connect(this, &RestorationWidget::start, m_dbusWorker, &DBusWorker::StartRestoration);
    connect(m_dbusWorker, &DBusWorker::RestorationProgressUpdate, this, [this] (int progress) {
        emit ProgressWidget::updateProgress(progress);
        if (progress == 100)
        {
            emit done();
        }
    });
    connect(m_dbusWorker, &DBusWorker::RestorationProgressUpdate, this, &RestorationWidget::updateDefaultEtaText);
}

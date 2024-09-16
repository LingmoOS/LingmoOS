// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QPixmap>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QDesktopWidget>

#include "upgradewidget.h"
#include "../mainwindow.h"
#include "systemupgradewidget.h"


UpgradeWidget::UpgradeWidget(QWidget *parent)
    : UpgradeProgressWidget(parent)
    , m_dbusWorker(DBusWorker::getInstance(this))
{
    initUI();
    initConnections();
}

void UpgradeWidget::initUI()
{
    m_iconLabel->setPixmap(QIcon(":/icons/system_upgrade.svg").pixmap(ICON_SIZE, ICON_SIZE));
    m_titleLabel->setText(tr("Upgrading…"));
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T4, QFont::Bold);
    m_tipLabel->setText(tr("Preparing for system upgrade, please wait patiently"));
    DFontSizeManager::instance()->bind(m_tipLabel, DFontSizeManager::T6, QFont::Medium);
}

void UpgradeWidget::initConnections()
{
    connect(this, &UpgradeWidget::start, m_dbusWorker, &DBusWorker::StartUpgradePreparation);
    connect(this, &UpgradeWidget::start, [this] {
        ProgressWidget::onStart();
    });
    connect(m_dbusWorker, &DBusWorker::UpgradePreparationUpdate, this, [this] (qint64 progress) {
        emit ProgressWidget::updateProgress(progress);
        if (progress == 100)
        {
            qCritical() << "upgradePreparationUpdate Done";
            emit done();
        }
    });
    connect(m_dbusWorker, &DBusWorker::UpgradePreparationUpdate, this, &UpgradeWidget::updateDefaultEtaText);
}

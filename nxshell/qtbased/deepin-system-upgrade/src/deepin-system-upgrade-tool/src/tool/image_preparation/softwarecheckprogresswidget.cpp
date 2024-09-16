// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DLabel>
#include <DPalette>

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QSizePolicy>

#include <string>
#include <unistd.h>

#include "softwarecheckprogresswidget.h"
#include "../../widgets/baselabel.h"
#include "../../widgets/simplelabel.h"


SoftwareCheckProgressWidget::SoftwareCheckProgressWidget(DWidget *parent)
    : DWidget(parent)
    , m_titleLabel(new DLabel(tr("Software Evaluation"), this))
    , m_dbusWorker(DBusWorker::getInstance(this))
    , m_mainVBoxLayout(new QVBoxLayout(this))
    , m_checklistVBoxLayout(new QVBoxLayout(this))
    , m_panelHBoxLayout(new QHBoxLayout(this))
    , m_checkProgress(new ProgressCircleWidget(this))
{
    initUI();
    initConnections();
}

void SoftwareCheckProgressWidget::initUI()
{
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T3, QFont::DemiBold);
    m_titleLabel->setForegroundRole(DPalette::TextTitle);

    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(1);

    DLabel *conditionsTextLabel = new DLabel;
    DFontSizeManager::instance()->bind(conditionsTextLabel, DFontSizeManager::T6, QFont::Normal);

    conditionsTextLabel->setForegroundRole(DPalette::TextTitle);
    conditionsTextLabel->setText(tr("Evaluate the compatibility of installed apps in the new system"));

    // Progress display widget
    m_checkProgress->setFixedSize(162, 162);
    m_checkProgress->setValue(1);
    m_checkProgress->start();

    m_mainVBoxLayout->setContentsMargins(0, 80, 0, 0);
    m_mainVBoxLayout->setSpacing(0);
    m_mainVBoxLayout->addWidget(m_checkProgress, 1, Qt::AlignCenter);
    m_mainVBoxLayout->addSpacing(20);
    m_mainVBoxLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    m_mainVBoxLayout->addSpacing(6);
    m_mainVBoxLayout->addWidget(conditionsTextLabel, 0, Qt::AlignCenter);
    m_mainVBoxLayout->addSpacing(90);

    setLayout(m_mainVBoxLayout);
}

void SoftwareCheckProgressWidget::initConnections()
{
    connect(this, SIGNAL(CheckDone()), this, SLOT(onCheckDone()));
    connect(m_dbusWorker, &DBusWorker::AppCheckProgressUpdate, this, &SoftwareCheckProgressWidget::onCheckProgressUpdate);
}

void SoftwareCheckProgressWidget::onCheckProgressUpdate(int progress)
{
    m_checkProgress->setValue(progress);
    if (progress == 100)
    {
        Q_EMIT CheckDone();
    }
}

void SoftwareCheckProgressWidget::onCheckDone()
{
    Q_EMIT AllCheckDone();
}

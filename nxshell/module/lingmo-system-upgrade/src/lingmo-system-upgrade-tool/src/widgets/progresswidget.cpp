// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QString>
#include <QDebug>

#include "progresswidget.h"
#include "../tool/mainwindow.h"

QString secondsToReadableTime(time_t sec)
{
    time_t hours = sec / 60. / 60;
    time_t minutes = (sec / 60) % 60;
    time_t seconds = sec % 60;
    return QString("%1:%2:%3").arg(
               QString::number(hours).rightJustified(2, '0'),
               QString::number(minutes).rightJustified(2, '0'),
               QString::number(seconds).rightJustified(2, '0')
           );
}

ProgressWidget::ProgressWidget(QWidget *parent)
    : QWidget(parent)
    , m_progressbar(new DProgressBar(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_progressbarLayout(new QHBoxLayout(this))
    , m_iconLabel(new SimpleLabel)
    , m_titleLabel(new SimpleLabel)
    , m_tipLabel(new SimpleLabel)
    , m_etaLabel(new SimpleLabel)
    , m_progressLabel(new SimpleLabel)
    , m_iconSpacerItem(new QSpacerItem(0, 10))
    , m_tipSpacerItem(new QSpacerItem(0, 56))
{
    initUI();
    initConnections();
}

void ProgressWidget::initUI()
{
    m_titleLabel->setForegroundRole(DPalette::TextTitle);
    m_tipLabel->setForegroundRole(DPalette::TextTips);
    m_etaLabel->setForegroundRole(DPalette::TextTips);

    m_progressbarLayout->setContentsMargins(0, 0, 0, 0);
    m_progressbarLayout->setSpacing(0);
    m_progressbarLayout->addSpacerItem(new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_progressbarLayout->addWidget(m_progressbar, 0, Qt::AlignCenter);
    m_progressbarLayout->addSpacerItem(new QSpacerItem(8, 0, QSizePolicy::Fixed));
    m_progressbarLayout->addWidget(m_progressLabel, 0, Qt::AlignCenter);
    m_progressbarLayout->addSpacerItem(new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_mainLayout->addSpacing(48);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacerItem(m_iconSpacerItem);
    m_mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(8);
    m_mainLayout->addWidget(m_tipLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacerItem(m_tipSpacerItem);
    m_mainLayout->addLayout(m_progressbarLayout, 1);
    m_mainLayout->addWidget(m_etaLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 114514, QSizePolicy::Minimum, QSizePolicy::Maximum));
}

void ProgressWidget::initConnections()
{
    connect(this, &ProgressWidget::updateProgress, this, &ProgressWidget::onUpdateProgress);
}

void ProgressWidget::onStart()
{
    MainWindow *window = MainWindow::getInstance();
    if (window->windowState() & Qt::WindowFullScreen)
    {
        // Setting progress bar length for fullscreen mode.
        m_progressbar->setFixedSize(270, 8);
        m_iconSpacerItem->changeSize(0, 40);
        m_tipSpacerItem->changeSize(0, 40);
    }
    else
    {
        // Setting progress bar length for fullscreen mode.
        m_progressbar->setFixedSize(400, 8);
    }

    // -1 is the uninitialized status. Due to resume downloading, the initial progress value have to be retrieved everytime a download is started.
    m_initProgress = -1;
    m_lastProgress = 0;
    // TODO(Yutao Meng): The time initialized here has some problems. It is recommended to be fixed in the later version.
    m_startTime = std::time(nullptr);
}

void ProgressWidget::onUpdateProgress(qint64 progress)
{
    m_progressbar->setValue(progress);
    m_progressLabel->setText(QString("%1%").arg(progress));
}

void ProgressWidget::updateDefaultEtaText(qint64 progress)
{
    if (m_lastProgress != progress)
    {
        m_lastProgress = progress;
        // TODO(Yutao Meng): Use milliseconds will improve the ETA time precision.
        QString etaTimeString = progress > m_initProgress ? secondsToReadableTime((time(nullptr) - m_startTime) * (100u - progress) / (progress - m_initProgress)) : tr("N/A");
        m_etaLabel->setText(tr("Time remaining: %1").arg(etaTimeString));
    }
}

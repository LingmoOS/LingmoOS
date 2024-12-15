// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "waterloadingwidget.h"

#include <QVBoxLayout>

WaterLoadingWidget::WaterLoadingWidget(QWidget *parent) : QWidget(parent)
{
    initUi();
    initConnection();
}

void WaterLoadingWidget::initUi()
{
    m_waterProgress = new DWaterProgress(this);
    m_waterProgress->setFixedSize(98, 98);
    m_waterProgress->setValue(0);

    m_time = new QTimer(this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_waterProgress, 0, Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);
}

void WaterLoadingWidget::initConnection()
{
    connect(m_time, &QTimer::timeout,this, &WaterLoadingWidget::onStartWaterProgress);
}

void WaterLoadingWidget::onStartWaterProgress()
{
    int value = (m_waterProgress->value() + qrand() % 10);
    value > 99 ? value = 99 : value;
    m_waterProgress->setValue(value);
}

void WaterLoadingWidget::setStartTime(int msec)
{
    m_waterProgress->start();
    m_waterProgress->setValue(1);
    m_time->start(msec);
}

void WaterLoadingWidget::stopTimer()
{
    m_waterProgress->setValue(100);
    m_waterProgress->stop();
    m_time->stop();
}



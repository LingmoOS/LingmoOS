// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "utilitylistdeviceitem.h"
#include "../corelib/helper.h"

#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>

UtilityListDeviceItem::UtilityListDeviceItem(QWidget *parent)
    : UtilityListItem(parent)
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    m_progressBar = new QProgressBar(widget);
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(false);
    m_sizeLabel = new QLabel(widget);
    m_sizeLabel->setObjectName("Title");

    layout->addWidget(m_sizeLabel, 0, Qt::AlignRight);
    layout->addSpacing(5);
    layout->addWidget(m_progressBar, 0, Qt::AlignRight);
    layout->setContentsMargins(0, 0, 30, 0);

    addWidget(widget, 0, Qt::AlignRight | Qt::AlignVCenter);
}

void UtilityListDeviceItem::setSizeInfo(qint64 used, qint64 total)
{
    m_sizeLabel->setText(QString("%1/%2").arg(Helper::sizeDisplay(used)).arg(Helper::sizeDisplay(total)));
    m_progressBar->setValue(100 * ((used / 1000.0) / (total / 1000.0)));
}

// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "formatingpage.h"

#include <DWaterProgress>

#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

DWIDGET_USE_NAMESPACE

FormatingPage::FormatingPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void FormatingPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    m_progressBox = new DWaterProgress(this);
    m_progressBox->setFixedSize(128,128);
    m_progressBox->start();
    m_progressBox->setValue(1);
    QLabel* formatingLabel = new QLabel(this);
    QString formatingText = tr("Formatting the disk, please wait...");
    formatingLabel->setText(formatingText);
    formatingLabel->setObjectName("StatusLabel");

    mainLayout->addSpacing(29);
    mainLayout->addWidget(m_progressBox, 0 , Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(formatingLabel, 0 , Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void FormatingPage::setProgress(double p)
{
//    qDebug() << "formating page progress = " << QString::number(int(p * 100));
    if (int(p * 100) < 1) {
        return;
    }
    m_progressBox->setValue(int(p * 100));
}

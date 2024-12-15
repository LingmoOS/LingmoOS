// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "warnpage.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

WarnPage::WarnPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void WarnPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QLabel* warnIconLabel = new QLabel(this);
    warnIconLabel->setPixmap(QIcon::fromTheme("dialog-information").pixmap(128, 128));
    QLabel* warnTextLabel = new QLabel(this);
    warnTextLabel->setWordWrap(true);
    QString warnText = tr("Formatting will erase all data on this disk, are you sure you want to continue? It cannot be restored.");
    warnTextLabel->setText(warnText);
    warnTextLabel->setWordWrap(true);
    warnTextLabel->setObjectName("StatusLabel");
    warnTextLabel->setAlignment(Qt::AlignHCenter);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(warnIconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(warnTextLabel, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
    setContentsMargins(20, 0, 20, 0);
}

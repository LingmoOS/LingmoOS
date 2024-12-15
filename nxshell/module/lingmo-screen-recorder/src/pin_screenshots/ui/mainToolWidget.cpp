// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainToolWidget.h"
#include "accessibility/acTextDefine.h"

MainToolWidget::MainToolWidget(DWidget *parent) : DWidget(parent)
{
    initMainToolWidget();
}

void MainToolWidget::initMainToolWidget()
{
    m_closeButton = new ToolButton(this);
    m_closeButton->setObjectName(AC_TOOLBARWIDGET_CLOSE_PIN_BUT);
    m_closeButton->setAccessibleName(AC_TOOLBARWIDGET_CLOSE_PIN_BUT);
    m_closeButton->setIcon(QIcon::fromTheme("close"));
    m_closeButton->setFixedSize(QSize(32, 32));
    m_closeButton->setIconSize(QSize(32, 32));
    connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(signalCloseButtonClicked()));

    m_saveButton = new ToolButton(this);
    m_saveButton->setObjectName(AC_MAINWINDOW_PIN_SAVE_BUT);
    m_saveButton->setAccessibleName(AC_MAINWINDOW_PIN_SAVE_BUT);
    m_saveButton->setIcon(QIcon::fromTheme("confirm"));
    m_saveButton->setFixedSize(QSize(32, 32));
    m_saveButton->setIconSize(QSize(32, 32));
    connect(m_saveButton, SIGNAL(clicked()), this, SIGNAL(signalSaveButtonClicked()));

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    hLayout->addWidget(m_closeButton, 0, Qt::AlignCenter);
    hLayout->addWidget(m_saveButton, 0, Qt::AlignCenter);
    this->setLayout(hLayout);
}

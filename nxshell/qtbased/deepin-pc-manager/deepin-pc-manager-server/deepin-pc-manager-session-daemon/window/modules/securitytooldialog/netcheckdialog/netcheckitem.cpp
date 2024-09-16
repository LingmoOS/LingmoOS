// Copyright (C) 2019 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netcheckitem.h"

#include <QHBoxLayout>

NetCheckItem::NetCheckItem(QString title, QWidget *parent)
    : DFrame(parent)
    , m_titleName(title)
    , m_title(nullptr)
    , m_tipTitle(nullptr)
    , m_spinner(nullptr)
{
    setFixedSize(QSize(680, 48));
    setLineWidth(0);
    setBackgroundRole(DPalette::ItemBackground);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(20, 10, 20, 10);
    setLayout(mainLayout);

    // 标题
    m_title = new DLabel(this);
    m_title->setAlignment(Qt::AlignLeft);
    m_title->setFixedWidth(280);
    m_title->setElideMode(Qt::ElideMiddle);
    mainLayout->addWidget(m_title, 0, Qt::AlignLeft);
    m_title->setText(m_titleName);

    // 说明
    m_tipTitle = new DLabel(this);
    m_tipTitle->setAlignment(Qt::AlignLeft);
    m_tipTitle->setFixedWidth(350);
    m_tipTitle->setElideMode(Qt::ElideMiddle);
    mainLayout->addWidget(m_tipTitle, 0, Qt::AlignLeft);
    m_currentColor = m_tipTitle->palette().text().color().name();

    // 等待转圈
    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(QSize(15, 15));
    m_spinner->hide();
    mainLayout->addWidget(m_spinner, 0, Qt::AlignRight);
}

NetCheckItem::~NetCheckItem()
{
}

// 重置
void NetCheckItem::resetNetCheckInfo()
{
    QPalette pal = m_tipTitle->palette();
    pal.setColor(QPalette::Text, QColor(m_currentColor));
    m_tipTitle->setPalette(pal);
    m_tipTitle->setText(tr("Awaiting detection"));
    m_spinner->hide();
}

// 开始检测
void NetCheckItem::startNetChecking()
{
    m_tipTitle->setText(tr("Detecting..."));
    m_spinner->start();
    m_spinner->show();
}

// 结束检测
void NetCheckItem::stopNetCheck(int status, QString result)
{
    if (status > CheckStatus::NoCheck) {
        QPalette pal = m_tipTitle->palette();
        if (CheckStatus::CheckFailed == status) {
            pal.setColor(QPalette::Text, Qt::red);
        } else {
            pal.setColor(QPalette::Text, Qt::green);
        }
        m_tipTitle->setPalette(pal);
    }

    m_tipTitle->setText(result);
    m_spinner->stop();
    m_spinner->hide();
}

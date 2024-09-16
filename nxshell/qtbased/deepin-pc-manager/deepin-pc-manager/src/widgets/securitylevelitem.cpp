// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitylevelitem.h"

#include <DApplicationHelper>
#include <DBackgroundGroup>

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

SecurityLevelItem::SecurityLevelItem(QWidget *parent)
    : DFrame(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);

    QWidget *firstLayerWidgt = new QWidget(this);
    mainLayout->addWidget(firstLayerWidgt);

    QHBoxLayout *firstLayerLayout = new QHBoxLayout;
    firstLayerLayout->setContentsMargins(10, 0, 10, 0);
    firstLayerWidgt->setLayout(firstLayerLayout);

    m_icon = new DLabel(this);
    firstLayerLayout->addWidget(m_icon);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(5, 9, 0, 9);
    firstLayerLayout->addLayout(rightLayout, Qt::AlignLeft);

    m_levelLabel = new DLabel(this);
    m_levelLabel->setAlignment(Qt::AlignLeft);
    m_levelLabel->setWordWrap(true);
    m_levelLabel->setContentsMargins(0, 0, 0, 0);
    QFont font = m_levelLabel->font();
    font.setBold(true);
    m_levelLabel->setFont(font);
    rightLayout->addWidget(m_levelLabel, 0, Qt::AlignLeft);
    rightLayout->setSpacing(0);

    m_tipLabel = new DTipLabel("", this);
    m_tipLabel->setAlignment(Qt::AlignLeft);
    m_tipLabel->setWordWrap(true);
    m_tipLabel->adjustSize();
    rightLayout->addWidget(m_tipLabel);

    m_statusIcon = new DLabel(this);
    firstLayerLayout->addWidget(m_statusIcon);

    setDisableStyle(false);
    setBackgroundRole(DPalette::ItemBackground);
    // 设置边框的宽度
    setLineWidth(0);
}

SecurityLevelItem::~SecurityLevelItem() { }

void SecurityLevelItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT clicked();
}

void SecurityLevelItem::setIcon(QIcon icon)
{
    m_icon->setPixmap(icon.pixmap(16, 20));
}

void SecurityLevelItem::setLevelText(QString text)
{
    m_levelLabel->setText(text);
}

void SecurityLevelItem::setTipText(QString text)
{
    m_tipLabel->setText(text);
}

void SecurityLevelItem::setDisableStyle(bool disable)
{
    if (disable) {
        QGraphicsOpacityEffect *opacityEffect[3];
        for (int i = 0; i < 3; i++) {
            opacityEffect[i] = new QGraphicsOpacityEffect(this);
            opacityEffect[i]->setOpacity(0.5);
        }

        m_icon->setGraphicsEffect(opacityEffect[0]);
        m_levelLabel->setGraphicsEffect(opacityEffect[1]);
        m_tipLabel->setGraphicsEffect(opacityEffect[2]);
        m_statusIcon->hide();
    } else {
        QGraphicsOpacityEffect *opacityEffect[3];
        for (int i = 0; i < 3; i++) {
            opacityEffect[i] = new QGraphicsOpacityEffect(this);
            opacityEffect[i]->setOpacity(1);
        }

        m_icon->setGraphicsEffect(opacityEffect[0]);
        m_levelLabel->setGraphicsEffect(opacityEffect[1]);
        m_tipLabel->setGraphicsEffect(opacityEffect[2]);
        m_statusIcon->show();
    }
}

void SecurityLevelItem::setStatusIcon(QIcon icon)
{
    m_statusIcon->setPixmap(icon.pixmap(16, 16));
}

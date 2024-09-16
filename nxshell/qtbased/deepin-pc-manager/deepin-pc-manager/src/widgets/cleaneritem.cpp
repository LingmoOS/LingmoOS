// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cleaneritem.h"

#include <DApplicationHelper>
#include <DBackgroundGroup>
#include <DFontSizeManager>

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

#define MAX_WIDTH 300

CleanerItem::CleanerItem(QWidget *parent)
    : DFrame(parent)
    , m_firstLayerWidgt(new QWidget(this))
    , m_checkBox(new DCheckBox(this))
    , m_levelLabel(new DLabel(this))
    , m_tipLabel(new DTipLabel("", this))
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    mainLayout->addWidget(m_firstLayerWidgt);

    QHBoxLayout *firstLayerLayout = new QHBoxLayout;
    firstLayerLayout->setContentsMargins(5, 0, 10, 0);
    firstLayerLayout->setSpacing(10);
    m_firstLayerWidgt->setLayout(firstLayerLayout);

    m_checkBox->setFixedSize(QSize(25, 25));
    firstLayerLayout->addWidget(m_checkBox);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(5, 5, 5, 0);
    rightLayout->setSpacing(5);
    firstLayerLayout->addLayout(rightLayout, Qt::AlignLeft);

    m_levelLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_tipLabel->adjustSize();
    m_levelLabel->setContentsMargins(0, 0, 0, 0);
    QFont font = m_levelLabel->font();
    font.setWeight(QFont::Medium);
    font.setPixelSize(14);
    m_levelLabel->setFont(font);

    QPalette pal;
    pal.setColor(QPalette::Text, QColor("#414D68"));
    m_levelLabel->setPalette(pal);
    rightLayout->addWidget(m_levelLabel, 0, Qt::AlignLeft);

    m_tipLabel->setAlignment(Qt::AlignLeft);
    m_tipLabel->adjustSize();
    m_tipLabel->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(m_tipLabel);

    QFont fontTip = m_tipLabel->font();
    fontTip.setPixelSize(12);
    m_tipLabel->setFont(fontTip);
    DFontSizeManager::instance()->unbind(m_tipLabel);

    QPalette pal1;
    pal1.setColor(QPalette::Text, QColor("#526A7F"));
    m_levelLabel->setPalette(pal1);
    setBackgroundRole(DPalette::ItemBackground);
    // 设置边框的宽度
    setLineWidth(0);

    connect(m_checkBox, &QCheckBox::clicked, this, &CleanerItem::setCheckBoxStatus);
}

CleanerItem::~CleanerItem() { }

// 设置相关标签名称
void CleanerItem::setAccessibleParentText(const QString &sAccessibleName)
{
    m_firstLayerWidgt->setAccessibleName(sAccessibleName + "_backGroundWidget");
    m_checkBox->setAccessibleName(sAccessibleName + "_backGroundWidget_checkBox");
    m_levelLabel->setAccessibleName(sAccessibleName + "_backGroundWidget_levelLable");
    m_tipLabel->setAccessibleName(sAccessibleName + "_backGroundWidget_tipLable");
}

// 设置标题和提示
void CleanerItem::setLevelText(QString text)
{
    m_levelLabel->setText(text);
}

void CleanerItem::setTipText(QString text)
{
    m_tipLabel->setText(text);
    m_tipStr = text;
}

// 设置/获取 勾选框状态
void CleanerItem::setCheckBoxStatus(bool isbStatus)
{
    m_checkBox->setChecked(isbStatus);
    Q_EMIT itemClicked();
}

bool CleanerItem::getCheckBoxStatus() const
{
    return m_checkBox->isChecked();
}

void CleanerItem::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontmet(m_tipLabel->font());
    QString elidedStr = fontmet.elidedText(m_tipStr, Qt::TextElideMode::ElideRight, MAX_WIDTH - 30);
    m_tipLabel->setText(elidedStr);
    DFrame::paintEvent(event);
}

// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pwdlimitlevelitem.h"

#include <DBackgroundGroup>
#include <DApplicationHelper>

#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QDebug>

DEF_NAMESPACE_BEGIN
PwdLimitLevelItem::PwdLimitLevelItem(QWidget *parent)
    : DFrame(parent)
    , m_icon(nullptr)
    , m_levelLabel(nullptr)
    , m_tipLabel(nullptr)
    , m_selectedIcon(nullptr)
    , m_notSelectIcon(nullptr)
    , m_selectIconLabel(nullptr)
    , m_canClick(true)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);
    // 设置布局大小改变策略
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_firstLayerWidgt = new QWidget(this);
    mainLayout->addWidget(m_firstLayerWidgt);

    QHBoxLayout *firstLayerLayout = new QHBoxLayout;
    firstLayerLayout->setContentsMargins(10, 0, 10, 0);
    m_firstLayerWidgt->setLayout(firstLayerLayout);

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

    // 初始化选中和没选中图标
    m_selectedIcon = QIcon();
    m_notSelectIcon = QIcon();
    // 初始化是否选中图标标签
    m_selectIconLabel = new DLabel(this);
    m_selectIconLabel->setPixmap(m_notSelectIcon.pixmap(16, 16));
    firstLayerLayout->addWidget(m_selectIconLabel);

    setBackgroundRole(DPalette::ItemBackground);
    // 设置边框的宽度
    setLineWidth(0);
}

PwdLimitLevelItem::~PwdLimitLevelItem()
{
}

// 设置相关标签名称
void PwdLimitLevelItem::setAccessibleParentText(QString sAccessibleName)
{
    m_firstLayerWidgt->setAccessibleName(sAccessibleName + "_backGroundWidget");
    m_levelLabel->setAccessibleName("backGroundWidget_levelLable");
    m_tipLabel->setAccessibleName("backGroundWidget_tipLable");
    m_selectIconLabel->setAccessibleName("backGroundWidget_selectIconLable");
}

void PwdLimitLevelItem::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if (m_canClick) {
        Q_EMIT clicked();
    }
}

void PwdLimitLevelItem::setIcon(const QIcon &icon)
{
    m_icon->setPixmap(icon.pixmap(16, 20));
}

void PwdLimitLevelItem::setLevelText(const QString &text)
{
    m_levelLabel->setText(text);
}

void PwdLimitLevelItem::setTipText(const QString &text)
{
    m_tipLabel->setText(text);
}

void PwdLimitLevelItem::setSelected(const bool &selected)
{
    QPixmap selectImg;
    qreal opacity;
    // 根据状态设置属性
    if (selected) {
        opacity = 1;
        selectImg = m_selectedIcon.pixmap(16, 16);
    } else {
        opacity = 0.5;
        selectImg = m_notSelectIcon.pixmap(16, 16);
    }
    QGraphicsOpacityEffect *opacityEffect[3];
    for (int i = 0; i < 3; i++) {
        opacityEffect[i] = new QGraphicsOpacityEffect(this);
        opacityEffect[i]->setOpacity(opacity);
    }
    // 设置透明度
    m_icon->setGraphicsEffect(opacityEffect[0]);
    m_levelLabel->setGraphicsEffect(opacityEffect[1]);
    m_tipLabel->setGraphicsEffect(opacityEffect[2]);
    // 设置是否选中图标
    m_selectIconLabel->setPixmap(selectImg);
}

void PwdLimitLevelItem::setEnable(const bool &enable)
{
    qreal opacity = 1;
    if (enable) {
        opacity = 1;
        m_canClick = true;
    } else {
        opacity = 0.5;
        m_canClick = false;
    }
    QGraphicsOpacityEffect *opacityEffect[4];
    for (int i = 0; i < 4; i++) {
        opacityEffect[i] = new QGraphicsOpacityEffect(this);
        opacityEffect[i]->setOpacity(opacity);
    }

    m_icon->setGraphicsEffect(opacityEffect[0]);
    m_levelLabel->setGraphicsEffect(opacityEffect[1]);
    m_tipLabel->setGraphicsEffect(opacityEffect[2]);
    m_selectIconLabel->setGraphicsEffect(opacityEffect[3]);
}
DEF_NAMESPACE_END

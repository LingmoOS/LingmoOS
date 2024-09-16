// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "basecontainerwidget.h"

BaseContainerWidget::BaseContainerWidget(QWidget *parent, int aDot)
    : QWidget(parent)
    , m_spacerItem(new QSpacerItem(0, 800))
    , m_buttonSpacerItem(new QSpacerItem(10, 0))
    , m_mainLayout(new QVBoxLayout(this))
    , m_contentLayout(new QVBoxLayout(this))
    , m_bottomUpperLayout(new QVBoxLayout(this))
    , m_bottomLayout(new QVBoxLayout(this))
    , m_buttonLayout(new QHBoxLayout(this))
    , m_cancelButton(new DPushButton(this))
    , m_midButton(new DPushButton(this))
    , m_suggestButton(new DSuggestButton(this))
    , m_threeDotsWidget(new ThreeDotsWidget(this, aDot))
{
    initUI();
}

void BaseContainerWidget::initUI()
{
    // Buttons are hidden by default.
    m_cancelButton->setVisible(false);
    m_midButton->setVisible(false);
    m_suggestButton->setVisible(false);

    m_buttonLayout->addSpacing(224);
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addSpacerItem(m_buttonSpacerItem);
    m_buttonLayout->addWidget(m_midButton);
    m_buttonLayout->addSpacerItem(m_buttonSpacerItem);
    m_buttonLayout->addWidget(m_suggestButton);
    m_buttonLayout->addSpacing(999);

    // Disable spacing
    m_mainLayout->setSpacing(0);
    m_bottomLayout->setSpacing(0);
    m_bottomUpperLayout->setSpacing(0);
    m_buttonLayout->setSpacing(0);
    m_contentLayout->setSpacing(0);

    m_mainLayout->addLayout(m_contentLayout);
    m_mainLayout->addItem(m_spacerItem);
    m_bottomLayout->addLayout(m_bottomUpperLayout);
    m_bottomLayout->addLayout(m_buttonLayout);
    m_bottomLayout->addSpacing(10);
    m_bottomLayout->addWidget(m_threeDotsWidget, 0, Qt::AlignCenter);
    m_mainLayout->addLayout(m_bottomLayout);
}

void BaseContainerWidget::addContentWidget(QWidget *content)
{
    m_contentLayout->addWidget(content);
}

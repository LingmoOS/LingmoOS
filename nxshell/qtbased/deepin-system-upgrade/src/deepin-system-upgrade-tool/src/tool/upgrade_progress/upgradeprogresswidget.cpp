// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "upgradeprogresswidget.h"

UpgradeProgressWidget::UpgradeProgressWidget(QWidget *parent)
    : ProgressWidget(parent)
{
    initUI();
    initConnections();
}

void UpgradeProgressWidget::initUI()
{
    m_titleLabel->setFont(QFont("SourceHanSansSC", 15, QFont::DemiBold));
    m_tipLabel->setFont(QFont("SourceHanSansSC", 10.5, QFont::Normal));
    m_tipLabel->setForegroundRole(DPalette::TextTitle);
    m_etaLabel->setFont(QFont("SourceHanSansSC", 9, QFont::Normal));
    m_etaLabel->setVisible(false);
    m_iconSpacerItem->changeSize(0, 0);
    m_tipSpacerItem->changeSize(0, 46);
}

void UpgradeProgressWidget::initConnections()
{
    connect(this, &UpgradeProgressWidget::start, this, &UpgradeProgressWidget::onStart);
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DPushButton>
#include <QVBoxLayout>
#include <QLabel>

#include "adjustmentatrriwidget.h"
#include "boxlayoutwidget.h"

AdjustmentAtrriWidget::AdjustmentAtrriWidget(QWidget *parent) : AttributeWgt(EImageAdaptScene, parent)
{
    initUi();
}

DToolButton *AdjustmentAtrriWidget::button()
{
    return  m_adjustmentBtn;
}

void AdjustmentAtrriWidget::setVar(const QVariant &var)
{
    bool bEnable = var.toBool();
    m_titleLabel->setEnabled(bEnable);
    m_adjustmentBtn->setEnabled(bEnable);
}

void AdjustmentAtrriWidget::initUi()
{
    m_adjustmentBtn = new DToolButton(nullptr);
    m_adjustmentBtn->setObjectName("PicFlipAdjustmentBtn");
    m_adjustmentBtn->setMaximumHeight(38);
    m_adjustmentBtn->setIcon(QIcon::fromTheme("autofit"));
    m_adjustmentBtn->setIconSize(QSize(36, 36));
    m_adjustmentBtn->setAlignment(Qt::AlignCenter);
    m_adjustmentBtn->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);

    m_adjustmentBtn->setText(tr("Auto fit"));
    m_adjustmentBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_adjustmentBtn->setFocusPolicy(Qt::NoFocus);

    m_titleLabel = new QLabel(tr("Auto fit"), this);
    m_titleLabel->setDisabled(true);

    QVBoxLayout *l = new QVBoxLayout;
    setLayout(l);

    l->addWidget(m_titleLabel);
    BoxLayoutWidget *w = new BoxLayoutWidget(this);
    w->addWidget(m_adjustmentBtn);
    l->addWidget(w);
}

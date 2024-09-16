// Copyright (C) 2022 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avcpulimitation.h"

#include <QCheckBox>
#include <QHBoxLayout>

AVCPULimitation::AVCPULimitation(QWidget *parent)
    : QWidget(parent)
    , m_context(new DTK_WIDGET_NAMESPACE::DLabel(this))
    , m_tips(new DTK_WIDGET_NAMESPACE::DLabel(this))
    , m_check(new QCheckBox(this))
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setAccessibleName("AVCPULimitation");
    // 布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    m_context->setText(tr("Energy-saving mode"));
    m_tips->setText("(" + tr("Virus scan takes a longer time in this mode") + ")");
    m_context->setElideMode(Qt::ElideMiddle);
    m_tips->setElideMode(Qt::ElideMiddle);

    layout->addWidget(m_check);
    layout->addWidget(m_context);
    layout->addWidget(m_tips);
    layout->addStretch(0);
    this->setLayout(layout);

    m_check->setTristate(false);
    m_check->setAccessibleName("mode_switch");

    connect(m_check, &QCheckBox::clicked, this, &AVCPULimitation::checkBoxChange);
}

AVCPULimitation::~AVCPULimitation() { }

void AVCPULimitation::setcheckbox(bool status)
{
    if (m_check->isChecked() == status) {
        return;
    }
    if (status) {
        m_check->setCheckState(Qt::Checked);
    } else {
        m_check->setCheckState(Qt::Unchecked);
    }
}

// 选择框的改变
void AVCPULimitation::checkBoxChange(bool ischecked)
{
    Q_EMIT notifyCheckBoxChange(ischecked);
}

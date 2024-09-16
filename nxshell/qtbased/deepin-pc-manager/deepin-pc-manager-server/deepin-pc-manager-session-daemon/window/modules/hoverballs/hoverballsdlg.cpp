// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "hoverballsdlg.h"

#include <QVBoxLayout>

HoverBallsDlg::HoverBallsDlg(QWidget *parent)
    : DAbstractDialog(parent)
    , m_waterProgress(nullptr)
{
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint |Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    // 初始化界面
    initUI();
}


void HoverBallsDlg::initUI()
{
    // 主布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 悬浮球
    m_waterProgress = new HoverBallsProgress(this);
    m_waterProgress->start();
    m_waterProgress->setValue(50);
    m_waterProgress->setFixedSize(QSize(50, 50));
    layout->addWidget(m_waterProgress);
}

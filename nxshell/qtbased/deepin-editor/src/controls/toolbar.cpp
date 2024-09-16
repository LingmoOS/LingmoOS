// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbar.h"
#include <QLabel>
#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE

ToolBar::ToolBar(QWidget *parent)
    : QWidget(parent),
      m_layout(new QHBoxLayout(this))
{
    m_layout->setContentsMargins(0, 0, 0, 0);
}

ToolBar::~ToolBar()
{
}

void ToolBar::setTabbar(QWidget *w)
{
    m_layout->addWidget(w, 0);
    m_layout->addSpacing(70);
}

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BaseWidget.h"

#include <DGuiApplicationHelper>

BaseWidget::BaseWidget(DWidget *parent)
    : DWidget(parent)
{
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

BaseWidget::~BaseWidget()
{

}

void BaseWidget::updateWidgetTheme()
{
    Dtk::Gui::DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(Dtk::Gui::DPalette::Background, plt.color(Dtk::Gui::DPalette::Base));
    setPalette(plt);
}

void BaseWidget::adaptWindowSize(const double &)
{

}

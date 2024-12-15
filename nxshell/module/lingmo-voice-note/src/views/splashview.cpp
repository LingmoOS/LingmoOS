// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "splashview.h"

#include <DLabel>
#include <DWaterProgress>
#include <DApplication>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

/**
 * @brief SplashView::SplashView
 * @param parent
 */
SplashView::SplashView(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);

    auto water = new DWaterProgress(this);
    water->setValue(50);
    water->setTextVisible(false);
    water->start();

    auto label = new DLabel(DApplication::translate("SplashView", "Loading..."));
    label->setObjectName("LoadWidgetLabel");
    label->setForegroundRole(DPalette::TextTitle);

    auto vbox = new QVBoxLayout;
    setLayout(vbox);

    vbox->addStretch();
    vbox->addWidget(water, 0, Qt::AlignCenter);
    vbox->addSpacing(10);
    vbox->addWidget(label, 0, Qt::AlignCenter);
    vbox->addStretch();
}

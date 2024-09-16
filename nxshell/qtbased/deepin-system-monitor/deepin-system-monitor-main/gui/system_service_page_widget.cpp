// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_service_page_widget.h"

#include "main_window.h"
#include "system_service_table_view.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DStyle>

#include <QHBoxLayout>
#include <QPainterPath>

// constructor
SystemServicePageWidget::SystemServicePageWidget(DWidget *parent)
    : DFrame(parent)
{
    // global app helper instance
    auto *dAppHelper = DApplicationHelper::instance();
    // global palette
    auto palette = dAppHelper->applicationPalette();

    // content margin
    int margin = 10;

    // main frame layout
    auto *layout = new QHBoxLayout(this);
    // service table view instance
    m_svcTableView = new SystemServiceTableView(this);
    layout->addWidget(m_svcTableView);
    layout->setContentsMargins(margin, margin, margin, margin);
    setLayout(layout);
}
// destructor
SystemServicePageWidget::~SystemServicePageWidget() {}

// paint event handler
void SystemServicePageWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QPainterPath path;
    path.addRect(QRectF(rect()));
    painter.setOpacity(1);

    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
    auto bgColor = palette.color(DPalette::Background);

    // paint frame background
    painter.fillPath(path, bgColor);
}

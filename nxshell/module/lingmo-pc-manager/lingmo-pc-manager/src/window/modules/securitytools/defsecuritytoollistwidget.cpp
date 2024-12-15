// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defsecuritytoollistwidget.h"

DefSecurityToolListWidget::DefSecurityToolListWidget(QWidget *parent)
    : DListWidget(parent)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
    setMovement(QListView::Static);
    setFrameShape(QListWidget::NoFrame);
    setEditTriggers(QListView::NoEditTriggers);
    setViewMode(QListView::IconMode);
    setResizeMode(QListView::Adjust);
    setFlow(QListView::LeftToRight);
    setUniformItemSizes(true);
    setDragEnabled(false);
    // setStyleSheet("QListWidget {border:1px solid gray;}");
    setContentsMargins(0, 0, 0, 0);
}

DefSecurityToolListWidget::~DefSecurityToolListWidget() { }

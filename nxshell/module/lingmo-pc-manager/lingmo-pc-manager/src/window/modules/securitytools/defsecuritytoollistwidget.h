// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLVIEW_H
#define DEFSECURITYTOOLVIEW_H

#include <DListWidget>

DWIDGET_USE_NAMESPACE

class DefSecurityToolListWidget : public DListWidget
{
    Q_OBJECT
public:
    explicit DefSecurityToolListWidget(QWidget *parent = nullptr);
    ~DefSecurityToolListWidget() override;

signals:

public slots:
};

#endif // DEFSECURITYTOOLVIEW_H

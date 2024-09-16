// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "hoverballsprogress.h"

#include <DWaterProgress>
#include <DAbstractDialog>
#include <DFrame>
#include <DDialog>

DWIDGET_USE_NAMESPACE
class HoverBallsDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit HoverBallsDlg(QWidget *parent = nullptr);

private:
    // 初始化界面
    void initUI();

private:
    HoverBallsProgress *m_waterProgress;
};

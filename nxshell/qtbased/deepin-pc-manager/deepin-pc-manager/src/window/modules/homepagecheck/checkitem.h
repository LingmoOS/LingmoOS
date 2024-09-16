// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "window/namespace.h"

#include <DCommandLinkButton>
#include <DFrame>
#include <DLabel>
#include <DSpinner>

class QHBoxLayout;

DWIDGET_USE_NAMESPACE

class CheckItem : public DFrame
{
    Q_OBJECT
public:
    CheckItem(DLabel *titlePic,
              DLabel *title,
              DLabel *tipPic,
              DLabel *tip,
              DCommandLinkButton *btn = nullptr,
              DCommandLinkButton *ignoreBtn = nullptr,
              DCommandLinkButton *notIgnoreBtn = nullptr,
              DSpinner *spinner = nullptr,
              QWidget *parent = nullptr);

private:
    // 标题图标
    DLabel *m_lbTitlePic;
    // 标题文字
    DLabel *m_lbTitle;
    // 提示图标
    DLabel *m_lbTipPic;
    // 提示文字
    DLabel *m_lbTip;
    // 操作按钮
    DCommandLinkButton *m_btn;
    // 忽略按钮
    DCommandLinkButton *m_ignoreBtn;
    // 取消忽略按钮
    DCommandLinkButton *m_notIgnoreBtn;
    // 旋转等待按钮
    DSpinner *m_spinner;
};

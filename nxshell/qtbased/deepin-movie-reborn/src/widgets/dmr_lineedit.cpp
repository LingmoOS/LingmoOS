// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file
 * 此文件为打开url时的输入框。
 */
#include "dmr_lineedit.h"

namespace dmr {
/**
 * @brief LineEdit 构造函数
 * @param parent 父窗口
 */
LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    //参考设计图,dtk默认最大36
    setMaximumHeight(36);

    QIcon icon;
    icon.addFile(":/resources/icons/input_clear_normal.svg", QSize(), QIcon::Normal);
    icon.addFile(":/resources/icons/input_clear_press.svg", QSize(), QIcon::Selected);
    icon.addFile(":/resources/icons/input_clear_hover.svg", QSize(), QIcon::Active);
    m_pClearAct = new QAction(icon, "", this);

    connect(m_pClearAct, &QAction::triggered, this, &QLineEdit::clear);
	connect(this, &QLineEdit::textChanged, this, &LineEdit::slotTextChanged);

}
/**
 * @brief showEvent 重载显示事件
 * @param pShowEvent 显示事件
 */
void LineEdit::showEvent(QShowEvent *pShowEvent)
{
    QLineEdit::showEvent(pShowEvent);
}
/**
 * @brief resizeEvent 重载界面大小改变事件
 * @param pResizeEvent 界面大小改变事件
 */
void LineEdit::resizeEvent(QResizeEvent *pResizeEvent)
{
    QLineEdit::resizeEvent(pResizeEvent);
}
/**
 * @brief slotTextChanged 文本变化槽函数
 * @param sText 输入框内的文本
 */
void LineEdit::slotTextChanged(const QString &sText)
{
    if (sText.isEmpty()) {
        setClearButtonEnabled(false);
    } else {
        setClearButtonEnabled(true);
    }
}

}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file
 * 此文件为打开url时的输入框。
 */
#pragma once

#include <QtWidgets>
#include <dlineedit.h>
#include <dimagebutton.h>

DWIDGET_USE_NAMESPACE

namespace dmr {
/**
 * @brief The LineEdit class
 * 这个类是播放网络视频输入url的输入栏
 */
class LineEdit: public QLineEdit {
public:
    /**
     * @brief LineEdit 构造函数
     * @param parent 父窗口
     */
    explicit LineEdit(QWidget* parent = 0);

protected:
    /**
     * @brief showEvent 重载显示事件
     * @param pShowEvent 显示事件
     */
    void showEvent(QShowEvent* pShowEvent) override;
    /**
     * @brief resizeEvent 重载界面大小改变事件
     * @param pResizeEvent 界面大小改变事件
     */
    void resizeEvent(QResizeEvent* pResizeEvent) override;
public slots:
    /**
     * @brief slotTextChanged 文本变化槽函数
     * @param sText 输入框内的文本
     */
    void slotTextChanged(const QString & sText);  //把lambda表达式改为槽函数，modify by myk
private:
    QAction *m_pClearAct;    ///输入框清空按键
};
}


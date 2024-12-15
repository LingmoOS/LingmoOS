// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSPARENTTEXTEDIT_H
#define TRANSPARENTTEXTEDIT_H

#include <QTextEdit>

#include <DWidget>

#include <QTimer>
#include <QContextMenuEvent>

DWIDGET_USE_NAMESPACE

/**
 * @brief The TransparentTextEdit class
 * 文本编辑控件
 */
class TransparentTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TransparentTextEdit(DWidget *parent = nullptr);

signals:
    /**
     * @brief sigNeedShowTips
     * 提示信号
     * @param tips
     */
    void sigNeedShowTips(const QString &tips, int);

    /**
     * @brief sigCloseNoteWidget 关闭注释编辑框窗口
     * @param isEsc 是否Esc键触发注释编辑框窗关闭
     */
    void sigCloseNoteWidget(bool isEsc = false);

private slots:
    /**
     * @brief slotTextEditMaxContantNum
     * 检测编辑框是否达到字数上限
     */
    void slotTextEditMaxContantNum();

protected:
    /**
     * @brief paintEvent
     * 绘制事件
     * @param event
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief insertFromMimeData
     * 重载剪贴板插入函数
     * @param source
     */
    void insertFromMimeData(const QMimeData *source) override;

    /**
     * @brief keyPressEvent
     * 键盘按键事件
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief focusOutEvent
     * 焦点丢失事件
     * @param event
     */
    void focusOutEvent(QFocusEvent *event) override;

private:
    int m_nMaxContantLen = 1500;  // 允许输入文本最大长度
};

#endif // TRANSPARENTTEXTEDIT_H

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORDWRAPLABEL_H
#define WORDWRAPLABEL_H

#include <DLabel>

DWIDGET_USE_NAMESPACE
/**
 * @brief The WordWrapLabel class
 * 自动换行Label
 */
class WordWrapLabel : public DLabel
{
public:
    explicit WordWrapLabel(QWidget *parent = nullptr);

    /**
     * @brief setText
     * 设置内容
     * @param text
     */
    void setText(const QString &text);

    /**
     * @brief setMargin
     * 设置间距
     * @param margin
     */
    void setMargin(int margin);

private:
    /**
     * @brief adjustContent
     * 自适应内容
     */
    void adjustContent();

protected:
    /**
     * @brief paintEvent
     * 绘制事件
     * @param event
     */
    void paintEvent(QPaintEvent *event);

private:
    QString m_text;
    int m_margin;
};

#endif // WORDWRAPLABEL_H

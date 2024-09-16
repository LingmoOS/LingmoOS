// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSWIDGET_H
#define TIPSWIDGET_H

#include <DWidget>

DWIDGET_USE_NAMESPACE
class TipsWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TipsWidget(QWidget *parent = nullptr);

    /**
     * @brief setText: set Tips text
     * @param text: text
     */
    void setText(const QString &text);

    /**
     * @brief setAlignment:set alignment
     */
    void setAlignment(Qt::Alignment);

private:
    /**
     * @brief initWidget: init widget layout
     */
    void initWidget();

    /**
     * @brief adjustContent: adjust content
     * @param text:content
     */
    void adjustContent(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onUpdateTheme();

private:
    QString m_text;   // text

    int m_lrMargin;   // left and right margin

    int m_tbMargin;   // top and bottom margin

    int m_maxLineCount;       // content lines count

    Qt::Alignment m_alignment;         // alignment
};

#endif // TIPSWIDGET_H

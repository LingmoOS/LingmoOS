// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORWIDGETACTION_H
#define COLORWIDGETACTION_H

#include <DWidget>

#include <QWidgetAction>

DWIDGET_USE_NAMESPACE

/**
 * @brief The ColorWidgetAction class
 * 颜色选项框Aciton
 */
class ColorWidgetAction : public QWidgetAction
{
    Q_OBJECT
    Q_DISABLE_COPY(ColorWidgetAction)

public:
    explicit ColorWidgetAction(DWidget *pParent = nullptr);

signals:
    /**
     * @brief sigBtnGroupClicked
     * 颜色按钮点击信号
     */
    void sigBtnGroupClicked();

private slots:
    /**
     * @brief slotBtnClicked
     * 响应颜色按钮点击
     */
    void slotBtnClicked(int);

private:
    /**
     * @brief initWidget
     * 控件初始化
     * @param pParent
     */
    void initWidget(DWidget *pParent);
};

#endif  // COLORWIDGETACTION_H

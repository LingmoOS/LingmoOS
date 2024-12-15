// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class BaseWidget : public DWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BaseWidget)

public:
    explicit BaseWidget(DWidget *parent = nullptr);
    virtual ~BaseWidget();

public:
    /**
     * @brief CustomWidget::adaptWindowSize
     * 缩略图列表自适应视窗大小
     * @param scale  缩放因子 大于0的数
     */
    virtual void adaptWindowSize(const double &);

protected:
    /**
     * @brief updateWidgetTheme
     * 主题变换更新
     */
    void updateWidgetTheme();

protected:
    bool bIshandOpenSuccess = false;
};

#endif  // CUSTOMWIDGET_H

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CentralNavPage_H
#define CentralNavPage_H

#include "BaseWidget.h"

/**
 * @brief The CentralNavPage class
 * 嵌入当前窗体中心控件的无文档默认页面
 */
class CentralNavPage : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CentralNavPage)

public:
    explicit CentralNavPage(DWidget *parent = nullptr);

signals:
    /**
     * @brief sigNeedOpenFilesExec
     * 请求阻塞式选择并打开文档
     */
    void sigNeedOpenFilesExec();

private slots:
    /**
     * @brief onThemeChanged
     * 主题变化时处理
     */
    void onThemeChanged();

};

#endif // OPENFILEWIDGET_H

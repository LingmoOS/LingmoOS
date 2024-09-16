// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include "partedproxy/dmdbushandler.h"
#include "devicelistwidget.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

class TitleWidget;
class MainSplitter;

/**
 * @class CenterWidget
 * @brief 主界面类
 */

class CenterWidget : public DWidget
{
    Q_OBJECT
public:
    explicit CenterWidget(DWidget *parent = nullptr);
    ~CenterWidget();

public:

    /**
     * @brief 退出服务
     */
    void HandleQuit();

    /**
     * @brief 获取功能图标按钮窗口
     * @return 返回窗口指针
     */
    TitleWidget *getTitleWidget();

private:

    /**
     * @brief 初始化页面
     */
    void initUi();

private:
    TitleWidget *m_titleWidget;
    MainSplitter *m_mainSpliter;
};

#endif // CENTERWIDGET_H

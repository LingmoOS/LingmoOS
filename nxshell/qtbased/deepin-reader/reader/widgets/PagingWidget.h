// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGINGWIDGET_H
#define PAGINGWIDGET_H

#include "BaseWidget.h"

#include <DIconButton>
#include <DLineEdit>
#include <DLabel>

class TMFunctionThread;
class DocSheet;

/**
 * @brief The PagingWidget class
 * 缩略图目录底部控件
 */
class PagingWidget : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PagingWidget)

public:
    explicit PagingWidget(DocSheet *sheet, DWidget *parent = nullptr);

    virtual void resizeEvent(QResizeEvent *event) override;

    ~PagingWidget() override;

    /**
     * @brief handleOpenSuccess
     * 文档打开成功,初始化相关数据
     */
    void handleOpenSuccess();

    /**
     * @brief setIndex
     * 设置指定页数
     * @param index
     */
    void setIndex(int index);

    /**
     * @brief setTabOrderWidget
     * 设置TAB切换控件顺序
     * @param tabWidgetlst
     */
    void setTabOrderWidget(QList<QWidget *> &tabWidgetlst);

private slots:
    /**
     * @brief slotPrePageBtnClicked
     * 上一页按钮点击响应
     */
    void slotPrePageBtnClicked();

    /**
     * @brief slotNextPageBtnClicked
     * 下一页按钮点击响应
     */
    void slotNextPageBtnClicked();

    /**
     * @brief slotUpdateTheme
     * 主题变更
     */
    void slotUpdateTheme();

    /**
     * @brief SlotJumpPageLineEditReturnPressed
     * 页数编辑框确认响应
     */
    void SlotJumpPageLineEditReturnPressed();

    /**
     * @brief onEditFinished
     * 页数编辑框编辑结束
     */
    void onEditFinished();

    /**
     * @brief FUNC任务执行完毕
     */
    void onFuncThreadFinished();

private:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

    /**
     * @brief setBtnState
     * 设置上一页,下一页按钮禁用状态
     * @param currntPage
     * @param totalPage
     */
    void setBtnState(const int &currntPage, const int &totalPage);

    /**
     * @brief normalChangePage
     * 跳转文档指定页数
     */
    void normalChangePage();

    /**
     * @brief pageNumberJump
     * 页面跳转
     */
    void pageNumberJump();

private:
    DLabel              *m_pTotalPagesLab = nullptr;        // 当前文档总页数标签
    DLabel              *m_pCurrentPageLab = nullptr;       // 当前文档当前页码
    DIconButton         *m_pPrePageBtn = nullptr;           // 按钮 前一页
    DIconButton         *m_pNextPageBtn = nullptr;          // 按钮 后一页
    DLineEdit           *m_pJumpPageLineEdit = nullptr;     // 输入框 跳转页码
    DocSheet            *m_sheet = nullptr;
    int                  m_curIndex = 0;

    bool                 m_bHasLabel = false;
    TMFunctionThread    *m_tmFuncThread = nullptr;
};

#endif // PAGINGWIDGET_H

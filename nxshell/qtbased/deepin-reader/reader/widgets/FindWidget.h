// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include "Application.h"

#include <DWidget>
#include <DFloatingWidget>
#include <DSearchEdit>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

class DocSheet;
/**
 * @brief The FindWidget class
 * 搜索窗口
 * 在父类右上角显示搜索框
 * 当父类resize时，会同步更新位置
 */
class FindWidget : public DFloatingWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FindWidget)

public:
    explicit FindWidget(DWidget *parent = nullptr);

    ~FindWidget() override;

public:
    /**
     * @brief setDocSheet
     * 设置文档对象
     * @param sheet
     */
    void setDocSheet(DocSheet *sheet);

    /**
     * @brief showPosition
     * 设置显示位置
     */
    void updatePosition();

    /**
     * @brief setSearchEditFocus
     * 设置搜索编辑框焦点
     */
    void setSearchEditFocus();

    /**
     * @brief setEditAlert
     * 搜索编辑框提示
     * @param iFlag
     */
    void setEditAlert(const int &iFlag);

    /**
     * @brief setYOff 设置y值的偏移
     * @param yOff
     */
    void setYOff(int yOff);
private slots:
    /**
     * @brief onCloseBtnClicked
     * 取消搜索并关闭
     */
    void onCloseBtnClicked();

    /**
     * @brief onSearchStop
     * 取消搜索
     */
    void onSearchStop();

    /**
     * @brief onSearchStart
     * 开始搜索
     */
    void onSearchStart();

    /**
     * @brief slotFindNextBtnClicked
     * 查找下一个
     */
    void slotFindNextBtnClicked();

    /**
     * @brief slotFindPrevBtnClicked
     * 查找上一个
     */
    void slotFindPrevBtnClicked();

    /**
     * @brief onTextChanged
     * 文本内容变化，如果空的时候需要设置提示
     */
    void onTextChanged();

private:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

protected:
    /**
     * @brief keyPressEvent
     * 键盘按键事件
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    DocSheet        *m_docSheet         = nullptr;
    DSearchEdit     *m_pSearchEdit      = nullptr;
    QString          m_lastSearchText; //上一次查找内容
    DIconButton     *m_findPrevButton   = nullptr;
    DIconButton     *m_findNextButton   = nullptr;
    QWidget         *m_parentWidget;
    int              m_yOff             = 0;//y值的偏移
};

#endif

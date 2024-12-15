// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCIENTIFICMODEL_H
#define SCIENTIFICMODEL_H

#include "../control/textbutton.h"
#include "../control/iconbutton.h"
#include "../control/scientifickeypad.h"
#include "../control/memhiskeypad.h"
#include "memhiswidget.h"
#include "sciexpressionbar.h"
#include "../views/memorywidget.h"
#include "../memorypublic.h"

#include <DWidget>
#include <DGuiApplicationHelper>

#include <QStackedWidget>
#include <QTimer>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

/**
 * @brief 科学模式界面
 */
class scientificModule : public DWidget
{
    Q_OBJECT

public:
    explicit scientificModule(QWidget *parent = nullptr);
    ~scientificModule();
    void setKeyPress(QKeyEvent *e);
    void checkLineEmpty();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *e);

signals:
    void changedeg(int deg);

public slots:
    //memory func
    void mAvailableEvent();
    void mUnAvailableEvent();
private slots:
    void initTheme(int type);
    void clickListView(const QModelIndex &index);
private:
    void handleEditKeyPress(QKeyEvent *);
    void handleKeypadButtonPress(int);
    void handleKeypadButtonPressByspace(int);
    void handleClearStateChanged(bool isAllClear);
    void handleDegChanged();
    void handleFEStateChanged(bool isdown);
    void handlePageStateChanged();
    void showMemHisWidget();
    void hideMemHisWidget();

private:
    QStackedWidget *m_stackWidget;
    SciExpressionBar *m_sciexpressionBar;
    MemHisKeypad *m_memhiskeypad;
    ScientificKeyPad *m_scikeypadwidget;
    MemHisWidget *m_memhiswidget;
    MemoryPublic *m_memoryPublic;
    bool m_avail = false; //内存是否有内容
    bool m_havail = false; //历史记录是否有内容
    int m_deg = 1;//1-deg 2-rad 3-grad
    bool m_FEisdown = false; //FE按键是否被按下
    bool m_Pageisdown = false; //2nd按键是否被按下
    bool m_isinsidefun = false;//鼠标点击在函数widget内
    bool m_memCalbtn; //m+,m-,ms
    bool m_memRCbtn;//mr,mc
    bool m_isallgray = false; //内存按钮全部置灰;
};

#endif // SCIENTIFICMODEL_H

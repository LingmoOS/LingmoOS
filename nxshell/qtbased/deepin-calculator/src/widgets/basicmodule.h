// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICMODULE_H
#define BASICMODULE_H

#include "inputedit.h"
#include "expressionbar.h"
#include "../control/textbutton.h"
#include "../control/iconbutton.h"
#include "../control/basickeypad.h"
#include "../control/memorykeypad.h"
#include "../views/memorywidget.h"
#include "../memorypublic.h"

#include <DWidget>
#include <DGuiApplicationHelper>

#include <QStackedWidget>
#include <QGridLayout>
#include <QTimer>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class BasicModule : public DWidget
{
    Q_OBJECT

public:
    explicit BasicModule(QWidget *parent = nullptr);
    ~BasicModule();

    void setKeyPress(QKeyEvent *e);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
public slots:
    //memory func
    void mAvailableEvent();
    void mUnAvailableEvent();
    void showListWidget();
    void closeListWidget();
private slots:
    void initTheme(int type);
private:
    void handleEditKeyPress(QKeyEvent *);
    void handleKeypadButtonPress(int);
    void handleKeypadButtonPressByspace(int);
    void handleClearStateChanged(bool isAllClear);
    void setBasicTabOrder();

protected:
    //void paintEvent(QPaintEvent *);

private:
    ExpressionBar *m_expressionBar;
    BasicKeypad *m_basicKeypad;
    MemoryKeypad *m_memoryKeypad;
    QStackedWidget *m_keypadLayout;
    MemoryWidget *m_memorylistwidget;
    bool m_insidewidget;
    bool m_avail = false; //内存中有数据
    bool m_isinsidefun = false;//鼠标点击在函数widget内
    bool m_memCalbtn; //m+,m-,ms可用
    bool m_memRCbtn;//mr,mc可用
    bool m_isallgray; //内存按钮全部置灰;
    MemoryPublic *m_memoryPublic;
};

#endif

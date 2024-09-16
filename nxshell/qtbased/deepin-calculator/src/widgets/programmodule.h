// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRAMMODULE_H
#define PROGRAMMODULE_H

#include "../views/memorywidget.h"
#include "../views/programmerarrowdelegate.h"
#include "../views/programmeritemwidget.h"
#include "../views/prolistview.h"
#include "../views/prolistmodel.h"
#include "../views/prolistdelegate.h"
#include "../control/bitbutton.h"
#include "../control/programmerkeypad.h"
#include "../control/procheckbtnkeypad.h"
#include "../control/prosystemkeypad.h"
#include "proexpressionbar.h"
#include "arrowrectangle.h"
#include "../memorypublic.h"

#include <DWidget>
#include <DGuiApplicationHelper>
#include <DArrowRectangle>
#include <DListWidget>
#include <DMenu>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class ProgramModule : public DWidget
{
    Q_OBJECT

public:
    explicit ProgramModule(QWidget *parent = nullptr);
    ~ProgramModule();
    void setKeyPress(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private slots:
    //数字键盘事件--鼠标点击、键盘交互、键盘输入
    void handleKeypadButtonPress(int key);
    void handleKeypadButtonPressByspace(int key);

    //arrowrectangle处理
    void shiftArrowListWidgetItemClicked(int row, bool isselect);
    void shiftArrowListWidgetItemSpace();
    void shiftArrowListWidgetItemPressed(int row);
    void byteArrowListWidgetItemClicked(int row, bool isselect);
    void byteArrowListWidgetItemSpace();
    void byteArrowListWidgetItemPressed(int row);

    void checkBtnKeypadThemeChange(int type);
    void radixListChange(const QModelIndex &index, bool isspace);

public slots:
    void mAvailableEvent();
    void mUnAvailableEvent();
    void showListWidget();
    void closeListWidget();

signals:
    void activateWindow();

private:
    void initArrowRectangle();
    void handleEditKeyPress(QKeyEvent *);
    void setwidgetAttribute(bool b);
    void handleClearStateChanged(bool isAllClear);
    void resetArrowWidth();

private:
    ProExpressionBar *m_proExpressionBar;
    ProListView *m_proListView;
    ProListModel *m_proListModel;
    ProListDelegate *m_proListDelegate;
    ProCheckBtnKeypad *m_checkBtnKeypad;
    ProgrammerKeypad *m_programmerKeypad;
    ProSystemKeypad *m_proSystemKeypad;
    QStackedWidget *m_stackWidget;
    MemoryPublic *m_memoryPublic;
    MemoryWidget *m_memorylistwidget;

    ArrowRectangle *m_byteArrowRectangle;
    MemoryListWidget *m_byteArrowListWidget;
    ProgrammerArrowDelegate *m_byteProgrammerArrowDelegate;
    ArrowRectangle *m_shiftArrowRectangle;
    MemoryListWidget *m_shiftArrowListWidget;
    ProgrammerArrowDelegate *m_shiftProgrammerArrowDelegate;

    //rectangle中当前选中的行数
    int m_shiftArrowCurrentRow = 0;
    int m_byteArrowCurrentRow = 0;

    //rectangle中当前点击的行数
    int m_shiftArrowPressRow = -1;
    int m_byteArrowPressRow = -1;

    //内存相关
    bool m_insidewidget = false;
    bool m_avail = false; //内存中有数据
    bool m_memCalbtn = false; //ms可用
    bool m_isallgray = false; //内存按钮全部置灰;
    int m_stackwidgetLastIndex = 0; //切换至内存前的页面
};

#endif // PROGRAMMODULE_H

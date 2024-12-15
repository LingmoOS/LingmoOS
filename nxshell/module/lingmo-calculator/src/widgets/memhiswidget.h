// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEMHISWIDGET_H
#define MEMHISWIDGET_H

#include "../control/iconbutton.h"
#include "../views/memorywidget.h"
#include "../views/simplelistdelegate.h"
#include "../views/simplelistmodel.h"
#include "../views/simplelistview.h"
#include "../memorypublic.h"

#include <DButtonBox>
#include <DIconButton>
#include <DWidget>
#include <DGuiApplicationHelper>

#include <QStackedWidget>
#include <QApplication>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

/**
 * @brief 科学模式历史记录及内存记录界面
 */
class MemHisWidget : public DWidget
{
    Q_OBJECT
public:
    /**
     * @brief 内存运算类型
     */
    enum memOperate {
        generateData, memoryplus, memoryminus, memoryclean, //MS, M+, M-, MC
        widgetplus, widgetminus //内存中的M+, M-
    };
    explicit MemHisWidget(QWidget *parent = nullptr);
    ~MemHisWidget();
    void focusOnButtonbox(Qt::FocusReason Reason);
    void memoryFunctions(memOperate operate = generateData, Quantity answer = Quantity(), int row = -1);
    void resetFocus();
    void historyfilled();
    MemoryWidget *getMemoryWiget();
    SimpleListModel* getSimpleListModel();

    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);
    void focusInEvent(QFocusEvent *event);

signals:
    void hisIsFilled(bool isfilled);
    void hideWidget();

public slots:
    void themeChanged(int type);
    void iconChanged(int type, int id);

private:
    QStackedWidget *m_stackWidget;
    SimpleListView *m_listView;
    SimpleListDelegate *m_listDelegate;
    SimpleListModel *m_listModel;
    MemoryWidget *m_memoryWidget;
    MemoryPublic *m_memoryPublic;
    DButtonBox *m_buttonBox;
    DButtonBoxButton *m_memoryBtn;
    DButtonBoxButton *m_historyBtn;
    IconButton *m_clearButton; //记录清除按钮
    bool m_isshowM = false; //内存中无内容为false
    bool m_isshowH = false;
    int m_themeType = 0; //当前主题
};

#endif // MEMHISWIDGET_H

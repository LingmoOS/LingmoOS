// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPRESSIONBAR_H
#define EXPRESSIONBAR_H

#include "../../3rdparty/core/evaluator.h"
#include "inputedit.h"
#include "../views/simplelistdelegate.h"
#include "../views/simplelistmodel.h"
#include "../views/simplelistview.h"

#include <DWidget>
#include <QKeyEvent>
#include <QPair>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

DWIDGET_USE_NAMESPACE

/**
 * @brief 存放联动信息
 */
struct historicalLinkageIndex {
    int linkageTerm;
    int linkedItem;
    QString linkageValue;
    bool isLink;
    historicalLinkageIndex()
    {
        linkageTerm = -1;
        linkedItem = -1;
        isLink = false;
    }
};

/**
 * @brief 暂未使用
 */
struct strck {
    QString text;
    bool isResult;
    strck() { isResult = false; }
};

class ExpressionBar : public DWidget
{
    Q_OBJECT

public:
    explicit ExpressionBar(QWidget *parent = nullptr);
    ~ExpressionBar();

    void mouseMoveEvent(QMouseEvent *e);

signals:
    void keyPress(QKeyEvent *);
    void clearStateChanged(bool);
    void themeChange(int type);
    void turnDeg();

public slots:
    //输入事件
    void enterNumberEvent(const QString &text);
    void enterSymbolEvent(const QString &text);
    void enterPointEvent();
    void enterBackspaceEvent();
    void enterClearEvent();
    void enterEqualEvent();
    void enterPercentEvent();
//    void enterPercentEventBak();
//    void enterPercentEventCommon();
    void enterBracketsEvent();
    void enterLeftBracketsEvent();
    void enterRightBracketsEvent();

    void moveLeft();
    void moveRight();
    void copyResultToClipboard();
    void copyClipboard2Result();
    void allElection();
    void shear();
    void deleteText();
    void clearLinkageCache(const QString &text, bool isequal);
//    void settingLinkage(const QModelIndex &index);
    void settingLinkage();
    void Undo();
    void addUndo();
    void Redo();
    void initTheme(int type);
//    void setSelection();
//    void getSelection();
    void setResultFalse();
    void replaceSelection(QString text);
    InputEdit *getInputEdit();

private slots:
    void handleTextChanged(const QString &text);
    void revisionResults(const QModelIndex &index);
    void onSeparateChange();//数字间隔位数发生改变

private:
    bool cursorPosAtEnd();
    QString formatExpression(const QString &text);
//    QString completedBracketsCalculation(QString &text);
    bool isOperator(const QString &text);
    bool cancelLink(int index);
    void initConnect();
    QString symbolComplement(const QString exp);
//    QString pasteFaultTolerance(QString exp);
    QString pointFaultTolerance(const QString &text);
    void expressionCheck();

private:
    Evaluator *m_evaluator;
    SimpleListView *m_listView;
    SimpleListDelegate *m_listDelegate;
    SimpleListModel *m_listModel;
    InputEdit *m_inputEdit;

    bool m_isContinue;        //点击结果左侧可继续输入，在结果最右侧清除结果
    bool m_isAllClear;         //C,AC切换标志
    bool m_isResult;           //计算结果
    bool m_isAutoComputation;  //自动计算
    bool m_inputNumber;        //输入数字
    bool m_isUndo;
    int m_hisRevision;  //历史记录修改项

    bool m_isLinked;     //联动状态
    int m_linkageIndex;  //联动索引缓存
    int m_Selected;      //历史记录选中项
    QVector<historicalLinkageIndex> m_hisLink;  //历史联动索引
    QVector<QString> m_undo;
    QVector<QString> m_redo;

    bool m_meanexp;    //是否表达式为纯数字
};

#endif

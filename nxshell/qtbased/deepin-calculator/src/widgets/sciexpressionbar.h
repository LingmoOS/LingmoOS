// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCIEXPRESSIONBAR_H
#define SCIEXPRESSIONBAR_H

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
#include <QToolTip>

DWIDGET_USE_NAMESPACE

/**
 * @brief 科学计算器表达式计算函数
 */
class SciExpressionBar : public DWidget
{
    Q_OBJECT

public:
    explicit SciExpressionBar(QWidget *parent = nullptr);
    ~SciExpressionBar();

    void mouseMoveEvent(QMouseEvent *e);

    QPair<bool, QString> getexpression();
    Quantity getanswer();

signals:
    void keyPress(QKeyEvent *);
    void clearStateChanged(bool);
    void themeChange(int type);
    void turnDeg();
    void fEStateChanged(bool);

public slots:
    //输入事件
    void enterNumberEvent(const QString &text);
    void enterSymbolEvent(const QString &text);
    void enterPointEvent();
    void enterBackspaceEvent();
    void enterClearEvent();
    void enterEqualEvent();
    void enterPercentEvent();
    void enterDegEvent(int mod);
    void enterFEEvent(bool isdown);
    void enterRandEvent();

    //replace
    void enterOperatorEvent(const QString &text); //运算符类：x A y;
    void enterFunctionEvent(const QString &text); //函数类：A(x),A(x,y);
    void enterConstantEvent(const QString &text); //常数类：e,pi,aˣ
    void enterBracketEvent(const int &type); //括号:0-(),1-(,2-)
    void enterSpecialFuncEvent(const QString &text); //倒数、绝对值、相反数

    void moveLeft();
    void moveRight();
    void copyResultToClipboard();
    void copyClipboard2Result();
    void allElection();
    void shear();
    void deleteText();
    void Undo();
    void addUndo();
    void Redo();
    void initTheme(int type);
    void setResultFalse();
    void replaceSelection(QString text);
    InputEdit *getInputEdit();
    void revisionResults(const QModelIndex &index);
    void hisRevisionResults(const QModelIndex &index, Quantity ans);
    bool judgeinput();

private slots:
    void handleTextChanged(const QString &text);
    /**
     * @brief pointCheckAfterDelete
     * 删除内容后5秒钟清理多余的小数点
     * @param curpos
     * 光标位置
     */
    void pointCheckAfterDelete();
    void onSeparateChange();//数字将位数发生改变

private:
    bool cursorPosAtEnd();
    QString formatExpression(const QString &text);
    bool isOperator(const QString &text);
    void initConnect();
    QString symbolComplement(const QString exp);
    QString pointFaultTolerance(const QString &text);
    void expressionCheck();
    bool isnumber(QChar a);
    bool expressionInFunc(QString &text);

private:
    Evaluator *m_evaluator;
    SimpleListView *m_listView;
    SimpleListDelegate *m_listDelegate;
    SimpleListModel *m_listModel;
    InputEdit *m_inputEdit;

    bool m_isContinue;
    bool m_isAllClear;
    bool m_isResult;           //计算结果
    bool m_inputNumber;        //输入数字
    bool m_isUndo;
    QVector<QString> m_undo;
    QVector<QString> m_redo;

    bool m_meanexp;    //是否表达式为纯数字
    bool m_FEisdown = false; //科学计算开关打开
    //支持的功能列表
    QList<QString> m_funclist;
    QString m_expression = QString();
    QPair<bool, QString> m_pair; //getexpression()函数的返回值，first为是否是错误表达式，second为表达式
    Quantity m_listanswer = Quantity(); //历史记录对应的answer
};
#endif // SCIEXPRESSIONBAR_H

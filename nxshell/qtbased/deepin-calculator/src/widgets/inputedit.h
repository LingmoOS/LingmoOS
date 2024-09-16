// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTEDIT_H
#define INPUTEDIT_H

#include "../../3rdparty/math/quantity.h"
#include "../../3rdparty/core/evaluator.h"

#include <DLineEdit>
#include <DPalette>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

struct SSelection {
    QString oldText; //选中时的当前text
    QString selected; //被选中部分
    QString clearText;
    bool isChanged;
    int curpos = 0;  // fix for bug-14249选中的左侧光标所在位置
    SSelection() { isChanged = false; }
};

class InputEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit InputEdit(QWidget *parent = nullptr);
    ~InputEdit();

    QString expressionText();
    void setAnswer(const QString &str, const Quantity &ans);
    void clear();
    SSelection getSelection() { return m_selected; }
    void setSelection(SSelection select) { m_selected = select; }
    QPair<bool, Quantity> getMemoryAnswer(); //edit 20200507,获取上一次计算的全精度结果，用于数字内存。
    QString symbolComplement(const QString exp);
    void getCurrentCursorPositionNumber(const int pos);//获取当前光标所在位置对应的数字并设置位键盘
    QString CurrentCursorPositionNumber(const int pos, const int base); //获取当前光标所在位置对应的数字
    QString CurrentCursorPositionNumber(const int pos);
    static bool isNumber(QChar a);//判断是否为数字(分隔符)
    QString formatBinaryNumber(const QString num);//清除二进制前多余的0
    static QString formatExpression(const int &probase, const QString &text);
    QPair<bool, Quantity> getCurrentAns();
    void focusInEvent(QFocusEvent *event);

public slots:
    void setUndoAction(bool state);
    void setRedoAction(bool state);
    QString symbolFaultTolerance(const QString &text);
    void isExpressionEmpty(); //edit 20200511,判断表达式是否有解，错误表达式无法存入内存  20200519 判断表达式是否为空
    void hisexpression(); //点击简易历史记录及科学左侧历史记录后清空ans
    void autoZoomFontSize(); //输入框字号变化
    void themetypechanged(int type);
    void valueChangeFromProSyskeypad(const QString num);
    void handleTextChanged(const QString &text);
    void radixChanged(int baseori, int basedest);
    QString scanAndExec(int baseori, int basedest);
    void onSwietThreeSeparateClicked();
    void onswietFourSeparateClicked();

Q_SIGNALS:
    void keyPress(QKeyEvent *);
    void equal();
    void undo();
    void redo();
    void paste();
    void copy();
    void cut();
    void selectAllText();
    void deleteText();
    void setResult();
    void emptyExpression(bool b);
    void cursorPositionNumberChanged(QString num);
    void prolistAns(QPair<bool, Quantity> pair);
    void swietThreeSeparate();  //切换为千分位
    void swietFourSeparate();  //切换为万分位
    void separateChange();

protected:
    void keyPressEvent(QKeyEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void initAction();
    void handleCursorPositionChanged(int oldPos, int newPos);
    void BracketCompletion(QKeyEvent *e);
    QString pointFaultTolerance(const QString &text);

    bool isSymbol(const QString &text);
    void multipleArithmetic(QString &text);
    void showTextEditMenu();
    void selectionChangedSlot();
    void showTextEditMenuByAltM();
    QString formatAns(const QString &text);

private:
    Quantity m_ans;
    int m_ansStartPos;
    int m_ansLength;
    bool m_ansVaild;
    bool m_currentInAns;  //光标在ans中间
    bool m_currentOnAnsLeft; //光标在ans左侧(包括左侧)
    QString m_oldText;
    SSelection m_selected;
    int m_lastPos; //最后进入计算时的光标位置

    QAction *m_undo;
    QAction *m_redo;
    QAction *m_cut;
    QAction *m_copy;
    QAction *m_paste;
    QAction *m_delete;
    QAction *m_select;
    QAction *m_threeSeparate;  //以三位分开
    QAction *m_fourSeparate;  //以四位分开

    Quantity m_memoryans; //用于内存的结果
    Evaluator *m_evaluator;
    bool m_ispercentanswer = false; //百分号结果是否需要转换为quantity
    //支持的功能列表
    QList<QString> m_funclist; //科学模式下函数名列表
    QString m_strans;
    //进制切换时用于替换所有数字
    QVector<QString> m_numvec;
    QVector<QString> m_opvec;
    QString m_textorder;
};

#endif

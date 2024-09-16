// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "textbutton.h"
#include "iconbutton.h"
#include "equalbutton.h"
#include "memorybutton.h"

#include <DWidget>
#include <DLabel>
#include <DGuiApplicationHelper>
#include <DSuggestButton>

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QDebug>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

/**
 * @brief 科学模式键盘区域界面
 */
class ScientificKeyPad : public DWidget
{
    Q_OBJECT

public:
    /**
     * @brief 科学模式键盘
     */
    enum Buttons {
        Key_deg, Key_page, Key_PI, Key_e, Key_Mod, Key_Backspace, //deg, 2ⁿᵈ, π, e, mod, 退格符
        Key_sin, Key_x2, Key_Derivative, Key_Factorials, Key_exp, Key_Clear, //sin, x², 1/x, x!, exp, C
        Key_cos, Key_x3, Key_left, Key_right, Key_Percent, Key_Div, //cos, x³, (, ), %, 除号
        Key_tan, Key_xy, Key_7, Key_8, Key_9, Key_Mult, //tan, xʸ, 7, 8, 9, 乘号
        Key_cot, Key_10x, Key_4, Key_5, Key_6, Key_Min, //cot, 10ˣ, 4, 5, 6, 减号
        Key_Modulus, Key_log, Key_1, Key_2, Key_3, Key_Plus, //|x|, log, 1, 2, 3, 加号
        Key_Rand, Key_ln, Key_opposite, Key_0, Key_Point, Key_Equals, //随机数, ln, +/-, 0, ., =
        Key_arcsin, Key_arccos, Key_arctan, Key_arccot, //arcsin, arccos, arctan, arccot
        Key_sqrt2, Key_sqrt3, Key_ysqrtx, Key_2x, Key_logyx, Key_ex //√x, ³√x, ʸ√x, 2ˣ, logᵧx, eˣ
    };

    explicit ScientificKeyPad(QWidget *parent = nullptr);
    ~ScientificKeyPad() override;

    void mouseMoveEvent(QMouseEvent *e) override;

    DPushButton *button(Buttons key);
    DPushButton *button(int key);
    void animate(Buttons key, bool isspace = false);
    bool buttonHasFocus();

    //科学模式第一页
    static const struct KeyDescription {
        QString text; //按钮text
        Buttons button; //按钮枚举值名
        int row; //行
        int column; //列
        int rowcount; //所占行数
        int columncount; //所占列数
    } keyDescriptions[];
    //科学模式第二页
    static const struct KeyDescription1 {
        QString text; //按钮text
        Buttons button; //按钮枚举值名
        int row; //行
        int column; //列
        int rowcount; //所占行数
        int columncount; //所占列数
    } keyDescriptions1[];

public slots:
    void initButtons();
    void buttonThemeChanged(int type);
    void turnPage(int key);
    void getdeg(int deg);
    void bracketsNum(int direction, QString num); //direction 0-左括号 1-右括号
    void getFocus(int);
signals:
    void buttonPressed(int);
    void buttonPressedbySpace(int);

private:
    void initUI();
    void initStackWidget(QStackedWidget *widget, DPushButton *button, DPushButton *pagebutton, const KeyDescription1 *desc);

private:
    QGridLayout *m_gridlayout1;
    QSignalMapper *m_mapper;
    DLabel *m_leftBracket; //存放左括号不匹配数
    DLabel *m_rightBracket; //存放右括号不匹配数
    QHash<Buttons, QPair<DPushButton *, const KeyDescription *>> m_keys;
    QHash<Buttons, QPair<DPushButton *, const KeyDescription1 *>> m_keys1;
    QPoint m_mousepoint;
    int m_themetype;
    int m_deg = 3; //1-deg  2-rad  3-grad
    QStackedWidget *m_arcsinwidget;
    QStackedWidget *m_arccoswidget;
    QStackedWidget *m_arctanwidget;
    QStackedWidget *m_arccotwidget;
    QStackedWidget *m_sqrtwidget;
    QStackedWidget *m_cbrtwidget;
    QStackedWidget *m_yrootwidget;
    QStackedWidget *m_2xwidget;
    QStackedWidget *m_logyxwidget;
    QStackedWidget *m_exwidget;
};

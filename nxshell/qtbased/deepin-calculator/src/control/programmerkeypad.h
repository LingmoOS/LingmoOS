// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRAMMERKEYPAD_H
#define PROGRAMMERKEYPAD_H

#include "textbutton.h"
#include "iconbutton.h"
#include "memorybutton.h"
#include "equalbutton.h"

#include <DGuiApplicationHelper>
#include <DSuggestButton>

#include <QWidget>
#include <QGridLayout>
#include <QSignalMapper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

/**
 * @brief 程序员模式普通键盘
 */
class ProgrammerKeypad : public DWidget
{
    Q_OBJECT

public:
    enum Buttons {
        Key_AND, Key_A, Key_moveL, Key_moveR, Key_Clear, Key_Backspace, //AND, A, <<, >>, AC, 退格
        Key_OR, Key_B, Key_leftBracket, Key_rightBracket, Key_percent, Key_Div, //OR, B, (, ), %, /
        Key_NOT, Key_C, Key_7, Key_8, Key_9, Key_Mult, //NOT, C, 7, 8, 9, *
        Key_NAND, Key_D, Key_4, Key_5, Key_6, Key_Min, //NAND, D, 4, 5, 6, -
        Key_NOR, Key_E, Key_1, Key_2, Key_3, Key_Plus, //NOR, E, 1, 2, 3, +
        Key_XOR, Key_F, Key_opposite, Key_0, Key_point, Key_equal //XOR, F, +/-, 0, ., =
    };

    explicit ProgrammerKeypad(QWidget *parent = nullptr);
    ~ProgrammerKeypad() override;

    void mouseMoveEvent(QMouseEvent *e) override;

    DPushButton *button(Buttons key);
    void animate(Buttons key, bool isspace = false);
    bool buttonHasFocus();

    static const struct KeyDescription {
        QString text; //按钮text
        Buttons button; //按钮枚举值名
        int row; //行
        int column; //列
    } keyDescriptions[];
public slots:
    void initButtons();
    void buttonThemeChanged(int type);
    void getFocus(int);
    void radixChanged(int row);
    void bracketsNum(int direction, QString num); //direction 0-左括号 1-右括号
signals:
    void buttonPressed(int);
    void buttonPressedbySpace(int);
    void equalPressed();

private:
    void initUI();

private:
    QGridLayout *m_layout;
    QSignalMapper *m_mapper;
    QHash<Buttons, QPair<DPushButton *, const KeyDescription *>> m_keys;
    QList<TextButton *> m_decdisable; //十进制置灰按钮
    QList<TextButton *> m_octdisable; //八进制置灰按钮
    QList<TextButton *> m_bindisable; //二进制置灰按钮
    int m_currentBase = 10;//当前进制

    DLabel *m_leftBracket; //存放左括号不匹配数
    DLabel *m_rightBracket; //存放右括号不匹配数
    int m_themetype;
};

#endif // PROGRAMMERKEYPAD_H

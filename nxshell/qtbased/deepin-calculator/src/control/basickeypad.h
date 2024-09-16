// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICKEYPAD_H
#define BASICKEYPAD_H

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

class BasicKeypad : public DWidget
{
    Q_OBJECT

public:
    /**
     * @brief 标准模式键盘
     */
    enum Buttons {
        Key_Clear, Key_Percent, Key_Backspace, Key_Div, //C, %, 退格符, 除号
        Key_7, Key_8, Key_9, Key_Mult, //7, 8, 9, 乘号
        Key_4, Key_5, Key_6, Key_Min, //4, 5, 6, 减号
        Key_1, Key_2, Key_3, Key_Plus, //1, 2, 3, 加号
        Key_0, Key_Point, Key_Brackets, Key_Equals //0, ., (), =
    };

    explicit BasicKeypad(QWidget *parent = nullptr);
    ~BasicKeypad() override;

    void mouseMoveEvent(QMouseEvent *e) override;

    DPushButton *button(Buttons key);
    DPushButton *button(int key);
    void animate(Buttons key, bool isspace = false);
    bool buttonHasFocus();

    static const struct KeyDescription {
        QString text; //按钮text
        Buttons button; //按钮枚举值名
        int row; //行
        int column; //列
        int rowcount; //所占行数
        int columncount; //所占列数
    } keyDescriptions[];
public slots:
    void initButtons();
    void buttonThemeChanged(int type);
    void getFocus(int);
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
};

#endif

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEMORYKEYPAD_H
#define MEMORYKEYPAD_H

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
 * @brief 简易计算器内存按键栏
 */
class MemoryKeypad : public DWidget
{
    Q_OBJECT

public:
    /**
     * @brief 内存按键栏
     */
    enum Buttons {Key_MC = 20, Key_MR, Key_Mplus, Key_Mminus, Key_MS, Key_Mlist}; //MC, MR, M+, M-, MS, M^

    explicit MemoryKeypad(QWidget *parent = nullptr);
    ~MemoryKeypad() override;

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
    void getFocus(int);
signals:
    void buttonPressed(int);
    void buttonPressedbySpace(int);
    void equalPressed();

private:
    QGridLayout *m_layout;
    QSignalMapper *m_mapper;
    QHash<Buttons, QPair<DPushButton *, const KeyDescription *>> m_keys;
};

#endif // MEMORYKEYPAD_H

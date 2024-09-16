// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MEMORYBUTTON_H
#define MEMORYBUTTON_H


#include "textbutton.h"
#include "../dsettings.h"

#include <DPalette>
#include <DPushButton>

#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHelpEvent>
#include <QToolTip>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

/**
 * @brief 标准模式内存按键
 */
class MemoryButton : public TextButton
{
    Q_OBJECT

public:
    MemoryButton(const QString &text = QString(), bool listwidgetbtn = false, QWidget *parent = nullptr); //listwidgetbtn-是否是内存界面中的按钮
    ~MemoryButton() override;

    void init();
    // void showShadow();
    // void hideShadow();
    void animate(bool isspace = false, int msec = 100);
    void setbtnlight(bool light);
    void showtips();
public slots:
    void setbuttongray(bool memorywidgetshow);
    void updateWhenBtnDisable();

public:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent *e) override;
    void focusOutEvent(QFocusEvent *) override;

private:
    QGraphicsDropShadowEffect *m_effect;
    DSettingsAlt *m_settings;

    DPalette m_palette;
    bool m_isHover;
    bool m_isPress;
    bool m_isgray; //按键不可用置灰
    QFont m_font;
    bool m_widgetbtn; //内存列表按键
    bool m_isallgray; //内存区显示置灰
    bool m_isacting = false; //规避鼠标在button内release时出现foucus状态的框
};

#endif  // MEMORYBUTTON_H

// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dtkwidget_global.h"
#include "window/namespace.h"

#include <DCheckBox>
#include <DFrame>
#include <DSwitchButton>
#include <DTipLabel>

#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class SafetyProtectionItem : public DFrame
{
    Q_OBJECT
public:
    explicit SafetyProtectionItem(QWidget *parent = nullptr);
    virtual ~SafetyProtectionItem();

    // 设置标题
    void setText(QString text);

    // 设置标题可以点击
    void setTextClick();

    // 设置开关状态
    virtual void setSwitchStatus(bool isbStatus);
    void enableSwitch(bool enable);

    // 设置相关标签名称
    void setAccessibleParentText(QString sAccessibleName);

    DSwitchButton *getSwitchBtn();
    // 复用
    // 不修改本类的情况下，改变layout与字体
    // 子类自行实现
    virtual void resetUI();

Q_SIGNALS:
    // 标题点击信号
    void sendTextClickedChange();

    // 点击
    void switchPressed();

    // 按钮状态转换
    void switchChanged(bool gStatus);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QWidget *m_firstLayerWidgt;
    DLabel *m_lbText;
    DLabel *m_lbTextNull;
    DSwitchButton *m_switchButton;
    bool m_bTestClick;
};

// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"
#include "dtkwidget_global.h"
#include "widgets/switchwidget.h"

#include <DFrame>
#include <DCheckBox>
#include <DTipLabel>

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
class SafetyProtectionItem : public DFrame
{
    Q_OBJECT
public:
    SafetyProtectionItem(QWidget *parent = nullptr);
    ~SafetyProtectionItem();

    // 设置标题
    void setText(QString text);

    // 设置标题可以点击
    void setTextClick();

    // 设置开关状态
    void setSwitchStatus(bool isbStatus);
    void enableSwitch(bool enable);

    // 设置相关标签名称
    void setAccessibleParentText(QString sAccessibleName);

private Q_SLOTS:

Q_SIGNALS:
    // 标题点击信号
    void sendTextClickedChange();

    // 点击
    void switchPressed();

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

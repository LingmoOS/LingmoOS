// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "window/namespace.h"

#include <DBackgroundGroup>
#include <DCommandLinkButton>
#include <DPushButton>

#include <QDebug>
#include <QLabel>
#include <QObject>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class FixItemWindow : public QFrame
{
    Q_OBJECT
public:
    FixItemWindow(bool flag, QWidget *parent = nullptr);
    void setIcon(QIcon icon);
    void setText(QString text);
    void setButtonText(QString text, bool flag);
    DCommandLinkButton *getButton();
    // 获得按钮的名字
    QString getButtonText();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    DBackgroundGroup *m_bgGroup;
    QLabel *m_leftIcon;
    QLabel *m_leftLabel;
    DCommandLinkButton *m_rightBtn;
    bool m_isFlag;
    //    DPushButton *m_rightBtn;
    //    DPushButton *m_rightBtn;
};

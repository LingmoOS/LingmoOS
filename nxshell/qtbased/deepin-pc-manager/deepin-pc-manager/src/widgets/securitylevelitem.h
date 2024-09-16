// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dtkwidget_global.h"
#include "window/namespace.h"

#include <DFrame>
#include <DTipLabel>

#include <QLabel>
#include <QTimer>
#include <QWidget>

DEF_NAMESPACE_BEGIN
DEF_ANTIVIRUS_NAMESPACE_BEGIN

DWIDGET_USE_NAMESPACE

class SecurityLevelItem : public DFrame
{
    Q_OBJECT
public:
    SecurityLevelItem(QWidget *parent = nullptr);
    ~SecurityLevelItem();

    void setIcon(QIcon icon);
    void setLevelText(QString text);
    void setTipText(QString text);
    void setDisableStyle(bool disable);
    void setStatusIcon(QIcon icon);

protected:
    void mousePressEvent(QMouseEvent *event);

Q_SIGNALS:
    void clicked();

private:
    DLabel *m_icon;
    DLabel *m_levelLabel;
    DTipLabel *m_tipLabel;
    DLabel *m_statusIcon;
};

DEF_ANTIVIRUS_NAMESPACE_END
DEF_NAMESPACE_END

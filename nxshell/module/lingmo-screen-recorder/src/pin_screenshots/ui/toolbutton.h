// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <DPushButton>
#include <DPalette>

#include <QApplication>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class ToolButton : public DPushButton
{
    Q_OBJECT
public:
    explicit ToolButton(DWidget *parent = 0)
    {
        Q_UNUSED(parent);
        m_hasHoverState = true;
        setCheckable(true);
        setFlat(true);
        this->setFocusPolicy(Qt::NoFocus);
    }
    void setHoverState(const bool hasHover)
    {
        m_hasHoverState = hasHover;
    }
    ~ToolButton() {}
protected:
    void enterEvent(QEvent *e) override
    {
        if (this->isEnabled() && m_hasHoverState) {
            setFlat(false);
        }
        DPushButton::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override
    {
        if (m_hasHoverState) {
            setFlat(true);
        }
        DPushButton::leaveEvent(e);
    }

private:
    bool m_hasHoverState;
};
#endif // TOOLBUTTON_H

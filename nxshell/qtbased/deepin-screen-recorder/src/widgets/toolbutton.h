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
    void setOptionButtonFlag(const bool flag)
    {
        m_isOptionButton = flag;
    }
    void setUndoButtonFlag(const bool flag)
    {
        m_isUndoButton = flag;
    }
    ~ToolButton() {}
protected:
    void enterEvent(QEvent *e) override
    {
        m_lastCursorShape = qApp->overrideCursor();
        qApp->setOverrideCursor(Qt::ArrowCursor);
        if (this->isEnabled() && m_hasHoverState) {
            setFlat(false);
        }
        if (m_isUndoButton) {
            //qDebug() << ">>>>>>>>>>>>>>>>>enterEvent";
            emit isInUndoBtn(true);
        }
        DPushButton::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override
    {
        if (m_hasHoverState) {
            setFlat(true);
        }
        qApp->setOverrideCursor(m_lastCursorShape->shape());
        DPushButton::leaveEvent(e);

        if (m_isMousePress) {
            qApp->setOverrideCursor(Qt::ArrowCursor);
            m_isMousePress = false;
        }

        if (m_isUndoButton) {
            //qDebug() << ">>>>>>>>>>>>>>>>>leaveEvent";
            emit isInUndoBtn(false);
        }
    }
    void mousePressEvent(QMouseEvent *e) override
    {
        if (m_isOptionButton) {
            m_isMousePress = true;
        }
        DPushButton::mousePressEvent(e);
    }
signals:
    /**
     * @brief isInButton 是否在button内部？
     * @param isUndo
     */
    void isInUndoBtn(bool isInUndo);
private:
    bool m_hasHoverState;
    bool m_isOptionButton = false;
    bool m_isUndoButton = false;
    bool m_isMousePress = false;
    QCursor *m_lastCursorShape = nullptr;
};
#endif // TOOLBUTTON_H

// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SLIDER_H
#define SLIDER_H

#include <DSlider>
#include <DWidget>

#include <QApplication>

DWIDGET_USE_NAMESPACE

class Slider : public DSlider
{
    Q_OBJECT
public:
    explicit Slider(DWidget *parent = 0)
    {
    }
    explicit Slider(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = nullptr)
    {
        DSlider(orientation, parent);
    }

    ~Slider() {}
protected:
    void enterEvent(QEvent *e) override
    {
        m_lastCursorShape = qApp->overrideCursor();
        qApp->setOverrideCursor(Qt::ArrowCursor);
        DSlider::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override
    {

        qApp->setOverrideCursor(m_lastCursorShape->shape());
        DSlider::leaveEvent(e);
    }

private:
    bool m_isMousePress = false;
    QCursor *m_lastCursorShape = nullptr;
};
#endif // SLIDER_H

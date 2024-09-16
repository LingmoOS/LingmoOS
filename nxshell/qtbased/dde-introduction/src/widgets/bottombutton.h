// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOTTOMBUTTON_H
#define BOTTOMBUTTON_H

#include <QRect>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

#include <DImageButton>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class BottomButton : public DImageButton
{
    Q_OBJECT
public:
    explicit BottomButton(DWidget *parent = nullptr);
    void setText(const QString text);
    void setRect(const QRect rect);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString         m_text;
    QRect           m_rect;
};

#endif // BOTTOMBUTTON_H

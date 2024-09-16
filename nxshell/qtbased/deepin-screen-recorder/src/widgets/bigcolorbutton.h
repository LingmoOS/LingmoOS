// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BIGCOLORBUTTON_H
#define BIGCOLORBUTTON_H

#include <DPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class BigColorButton : public DPushButton
{
    Q_OBJECT
public:
    explicit BigColorButton(DWidget *parent = 0);
    ~BigColorButton();

    void setColor(QColor color);
    //void setColorIndex();
    void setCheckedStatus(bool checked);
    void updateConfigColor(const QString &shape,  const QString &key, int index);

protected:
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

private:
    QColor m_color;
    bool m_isHover;
    bool m_isChecked;
};

#endif // BIGCOLORBUTTON_H

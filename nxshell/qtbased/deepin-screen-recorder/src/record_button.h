// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDBUTTON_H
#define RECORDBUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE

class RecordButton : public DPushButton
{
    Q_OBJECT

    static const int WIDTH;
    static const int HEIGHT;
    static const int TEXT_PADDING;

public:
    explicit RecordButton(DPushButton *parent = nullptr);
    void setText(QString string);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *, QEvent *event);

private:
    QPixmap normalImg;
    QPixmap hoverImg;
    QPixmap pressImg;

    bool isFocus;
    bool isPress;

    QString text;
    QSize textSize;
};

#endif // RECORDBUTTON_H

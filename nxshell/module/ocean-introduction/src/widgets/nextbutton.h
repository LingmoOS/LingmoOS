// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NEXTBUTTON_H
#define NEXTBUTTON_H

#include <QPushButton>
#include <QPainterPath>
#include <QPainter>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>

#include <DPushButton>

DWIDGET_USE_NAMESPACE

class NextButton : public DPushButton
{
    Q_OBJECT
public:
    explicit NextButton(const QString &text, QWidget *parent = nullptr);

    //Normal 正常　Transparent 透明
    enum Mode {
        Normal,
        Transparent
    };

    void setMode(Mode mode);

public:
    //void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *) override;

private:
    QString m_text;
    Mode m_mode;
    QColor m_currentColor;
};

#endif // NEXTBUTTON_H

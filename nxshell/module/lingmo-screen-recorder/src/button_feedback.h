// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUTTONFEEDBACK_H
#define BUTTONFEEDBACK_H

#include <DWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE

class ButtonFeedback : public DWidget
{
    Q_OBJECT

    static const int FRAME_RATE;

public:
    explicit ButtonFeedback(DWidget *parent = nullptr);
    virtual ~ButtonFeedback();

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void showPressFeedback(int x, int y);
    void showDragFeedback(int x, int y);
    void showReleaseFeedback(int x, int y);
    void update();

private:
    QPixmap buttonFeedbackImg[10];

    QTimer *timer;

    int frameIndex;
    QPainter* m_painter = nullptr;
};

#endif

// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "button_feedback.h"
#include "utils.h"

#include <DHiDPIHelper>
#include <DWindowManagerHelper>

#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QBitmap>
#include <QPaintEvent>
#include <QDebug>

const int ButtonFeedback::FRAME_RATE = 40; // ms

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

ButtonFeedback::ButtonFeedback(DWidget *parent) : DWidget(parent)
{
    installEventFilter(this);  // add event filter
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    for (int i = 0; i < 10; ++i) {
        QString svgName = QString("recorder/button_feedback_%1.svg").arg(i);
        buttonFeedbackImg[i] = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath(svgName));
    }
    /*
    buttonFeedback0Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_0.svg"));
    buttonFeedback1Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_1.svg"));
    buttonFeedback2Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_2.svg"));
    buttonFeedback3Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_3.svg"));
    buttonFeedback4Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_4.svg"));
    buttonFeedback5Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_5.svg"));
    buttonFeedback6Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_6.svg"));
    buttonFeedback7Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_7.svg"));
    buttonFeedback8Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_8.svg"));
    buttonFeedback9Img = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("button_feedback_9.svg"));
    */
    setFixedSize(buttonFeedbackImg[0].width(), buttonFeedbackImg[0].height());

    frameIndex = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    Utils::passInputEvent(static_cast<int>(this->winId()));
    m_painter =  new QPainter();
}

ButtonFeedback::~ButtonFeedback()
{
    delete m_painter;
    if (nullptr != timer) {
        delete  timer;
        timer = nullptr;
    }
}

void ButtonFeedback::update()
{
    repaint();

    frameIndex += 1;

    if (frameIndex > 9) {
        frameIndex = 0;

        if (timer->isActive()) {
            timer->stop();
        }

        hide();
    }
}

void ButtonFeedback::paintEvent(QPaintEvent *event)
{
    /*
    QPainter painter(this);
    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::transparent);
    painter.begin( &pixmap );
    painter.setRenderHints( QPainter::Antialiasing, true);



    painter.setOpacity(1);
    painter.drawPixmap(QPoint(0, 0), buttonFeedbackImg[frameIndex]);
    painter.end();

    painter.begin(this);
    painter.drawPixmap(QPoint(0, 0), pixmap);
    painter.end();
    //setMask(pixmap.mask());
    //event->accept();
    */

    QPixmap pixmap(width() + 6, height() + 6);
    pixmap.fill(Qt::transparent);
    m_painter->begin(&pixmap);
    m_painter->setRenderHints(QPainter::Antialiasing, true);

    m_painter->setOpacity(1);
    m_painter->drawPixmap(QPoint(0, 0), buttonFeedbackImg[frameIndex]);
    m_painter->end();

    m_painter->begin(this);
    m_painter->drawPixmap(QPoint(0, 0), pixmap);
    m_painter->end();
    //setMask(pixmap.mask());
    if (!DWindowManagerHelper::instance()->hasComposite()) {
        setMask(pixmap.mask());
    }
    event->accept();
}

void ButtonFeedback::showPressFeedback(int x, int y)
{
    frameIndex = 1;

    show();
    repaint();
    QPoint dpos = Utils::getPosWithScreen(QPoint(x, y));
    qreal devicePixelRatio = qApp->devicePixelRatio();
    int dx = static_cast<int>(dpos.x() - rect().width() / devicePixelRatio / 2);
    int dy = static_cast<int>(dpos.y() - rect().height() / devicePixelRatio / 2);
    move(dx, dy);
    timer->start(FRAME_RATE);
}

void ButtonFeedback::showDragFeedback(int x, int y)
{
    frameIndex = 2;

    show();
    repaint();
    QPoint dpos = Utils::getPosWithScreen(QPoint(x, y));
    qreal devicePixelRatio = qApp->devicePixelRatio();
    int dx = static_cast<int>(dpos.x() - rect().width() / devicePixelRatio / 2);
    int dy = static_cast<int>(dpos.y() - rect().height() / devicePixelRatio / 2);
    move(dx, dy);
    if (timer->isActive()) {
        timer->stop();
    }
}

void ButtonFeedback::showReleaseFeedback(int x, int y)
{
    frameIndex = 3;

    show();
    repaint();
    QPoint dpos = Utils::getPosWithScreen(QPoint(x, y));
    qreal devicePixelRatio = qApp->devicePixelRatio();
    int dx = static_cast<int>(dpos.x() - rect().width() / devicePixelRatio / 2);
    int dy = static_cast<int>(dpos.y() - rect().height() / devicePixelRatio / 2);
    move(dx, dy);
    timer->start(FRAME_RATE);
}

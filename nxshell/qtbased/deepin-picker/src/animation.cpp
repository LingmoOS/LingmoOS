// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "animation.h"
#include "utils.h"
#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QDebug>
#include <QPainterPath>

Animation::Animation(int x, int y, QPixmap pixmap, QColor color, QWidget *parent) : QWidget(parent)
{
    // Init window flags to make window transparent and get correctly behavior.
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
//    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
//    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);
    installEventFilter(this);

    // Init attributes.
    animationDuration = 25;
    animationFrames = 12;
    blockHeight = 20;
    blockWidth = 20;
    cursorColor = color;
    cursorX = x;
    cursorY = y;
    height = 220;
    renderTicker = 0;
    screenshotPixmap = pixmap;
    width = 220;

    // Move and resize window.
    move(x - width / 2, y - height / 2);
    resize(width, height);

    // Start animation when module init.
    renderTimer = new QTimer();
    connect(renderTimer, &QTimer::timeout, this, &Animation::renderAnimation);
    renderTimer->start(animationDuration);
}

Animation::~Animation()
{
    delete renderTimer;
}

void Animation::paintEvent(QPaintEvent *)
{
    // Make clip radius change along with ticker.
    qreal devicePixelRatio = qApp->devicePixelRatio();
    int radius = screenshotPixmap.width() / 2 * (1 - Utils::easeInOut(renderTicker * 1.0 / animationFrames));
    radius = radius / devicePixelRatio;

    int width = rect().width();
    int height = rect().height();

    // Init painter.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Make opacity change along with ticker.
    painter.setOpacity(Utils::easeInOut(renderTicker * 1.0 / animationFrames));

    // Draw screenshot pixmap.
    QPainterPath circlePath;
    circlePath.addEllipse(width / 2 - radius, height / 2 - radius, radius * 2, radius * 2);
    painter.setClipPath(circlePath);
    painter.drawPixmap(width / 2 - radius, height / 2 - radius, screenshotPixmap);
}

void Animation::renderAnimation()
{
    if (renderTicker < animationFrames) {
        renderTicker++;

        repaint();
    } else {
        renderTimer->stop();
        hide();                 // hide window when animation finish

        emit finish();
    }
}


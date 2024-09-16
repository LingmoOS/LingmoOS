// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DWidget>
#include <QPainter>
#include <QDesktopWidget>
#include <QApplication>
#include <QStyle>
#include <QEvent>
#include "start_tooltip.h"
#include "utils.h"
#include "constant.h"
#include <DHiDPIHelper>

#include <QDebug>

DWIDGET_USE_NAMESPACE

StartTooltip::StartTooltip(DWidget *parent) : DWidget(parent)
{
    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    iconImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("deepin-screen-recorder.svg"));

    installEventFilter(this);

    text = tr("Click or drag to\nselect the area to record");
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, text);

    qreal devicePixelRatio = qApp->devicePixelRatio();
    setFixedSize(size.width() + Constant::RECTANGLE_PADDING * 2,
                 size.height() + iconImg.height() / devicePixelRatio + Constant::RECTANGLE_PADDING * 3);

    Utils::passInputEvent(this->winId());
}

void StartTooltip::setWindowManager(DWindowManager *wm)
{
    windowManager = wm;

    const qreal ratio = devicePixelRatioF();
    WindowRect rootWindowRect = windowManager->getRootWindowRect();
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            QRect(rootWindowRect.x * ratio, rootWindowRect.y * ratio, rootWindowRect.width, rootWindowRect.height)
        )
    );
}

void StartTooltip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    Utils::drawTooltipBackground(painter, rect());

    const qreal devicePixelRatio = devicePixelRatioF();
    painter.setOpacity(1);
    painter.drawPixmap(QPoint((rect().width() - iconImg.width() / devicePixelRatio) / 2, Constant::RECTANGLE_PADDING / devicePixelRatio), iconImg);

    Utils::drawTooltipText(painter, text, "#000000", Constant::RECTANGLE_FONT_SIZE,
                           QRectF(rect().x() * devicePixelRatio,
                                  rect().y() * devicePixelRatio + Constant::RECTANGLE_PADDING / devicePixelRatio + iconImg.height() / devicePixelRatio,
                                  rect().width(),
                                  rect().height() - Constant::RECTANGLE_PADDING / devicePixelRatio - iconImg.height() / devicePixelRatio
                                 ));
}

bool StartTooltip::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::ShowToParent) {
        Utils::blurRect(windowManager, this->winId(), rect());
    } else if (event->type() == QEvent::HideToParent) {
        Utils::clearBlur(windowManager, this->winId());
    }

    return false;
}

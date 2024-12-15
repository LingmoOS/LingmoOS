// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resultpopup.h"
#include "closepopup.h"

#include <DHiDPIHelper>

#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

DGUI_USE_NAMESPACE
Resultpopup::Resultpopup(bool compositing, QWidget *parent)
    : DWidget(parent)
    , compositingStatus(compositing)
    , resultInfo(new Resultinfo)
    , buttonRest(new Buttonrest)
    , buttonAgain(new Buttonagain)
{
    //设置大小
    setFixedSize(1024, 768);

    initBackgroundPix();

    //休息一下, 关闭弹窗
    connect(buttonRest, &Buttonrest::signalButtonRestClicked, this, [ = ] {
        this->close();
        emit signalHaveRest();
    });
    //再来一次,发送重玩信号,关闭弹窗
    connect(buttonAgain, &Buttonagain::signalButtonAnginClicked, this, [ = ] {
        this->close();
        emit signalGameAgain();
    });
}

/**
 * @brief Resultpopup::initUI 初始化界面
 */
void Resultpopup::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();

    QHBoxLayout *closeLayout = new QHBoxLayout();
    Closepopup *closeBt = new Closepopup();
    connect(closeBt, &Closepopup::signalCloseClicked, this, [ = ] {
        this->close();
        emit signalHaveRest();
    });
    closeLayout->addWidget(closeBt, 0, Qt::AlignRight);
    closeLayout->addSpacing(328); //关闭按钮到右侧边界距离

    QHBoxLayout *labelLayout = new QHBoxLayout();
    resultInfo->setMinimumWidth(this->width());
    labelLayout->addWidget(resultInfo, 0, Qt::AlignHCenter);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addSpacing(340); //按钮到左侧边界距离
    buttonLayout->addWidget(buttonRest);
    buttonLayout->addWidget(buttonAgain);
    buttonLayout->addSpacing(340); //按钮到右侧边界距离

    mainLayout->addSpacing(274); //关闭按钮到上侧边界距离
    mainLayout->addLayout(closeLayout);
    mainLayout->addSpacing(34); //关闭按钮和文字之间距离
    mainLayout->addLayout(labelLayout);
    mainLayout->addSpacing(24); //文字和按钮之间距离
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(297); //按钮到底部边界距离
    setLayout(mainLayout);
}

/**
 * @brief Resultpopup::initBackgroundPix 初始化背景图片
 */
void Resultpopup::initBackgroundPix()
{
    //根据是否有特效设置不同图片
    if (compositingStatus) {
        winPixmap = DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/background_win.svg");
        failPixmap = DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/background_fail.svg");
    } else {
        winPixmap = DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/background_win_nshadow.svg");
        failPixmap = DHiDPIHelper::loadNxPixmap(":/resources/resultpopup/background_fail_nshadow.svg");
    }
}

/**
 * @brief Resultpopup::popupShow 展示弹窗
 */
void Resultpopup::popupShow()
{
    this->show();
}

/**
 * @brief setHasWin 设置输赢
 * @param win 是否赢了
 */
void Resultpopup::setHasWin(bool win)
{
    hasWin = win;
    resultInfo->setResult(hasWin);
    buttonRest->setResult(hasWin);
    buttonAgain->setResult(hasWin);

    initBackgroundPix();
    initUI();
}

/**
 * @brief Resultpopup::popupClose 关闭弹窗 发出信号关闭事件循环
 */
void Resultpopup::popupClose()
{
    this->close();
    emit signalHaveRest();
}

/**
 * @brief Resultpopup::slotCompositingChanged 特效窗口
 * @param compositing 是否开启
 */
void Resultpopup::slotCompositingChanged(bool compositing)
{
    compositingStatus = compositing;
    initBackgroundPix();
    update();
}

/**
 * @brief Resultpopup::paintEvent 绘图事件
 * @param event
 */
void Resultpopup::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPixmap backgroundImage;
    if (hasWin) {
        backgroundImage = winPixmap;
    } else {
        backgroundImage = failPixmap;
    }
    painter.setPen(Qt::NoPen);
    //背景阴影
    QColor tColor = "#000000";
    tColor.setAlphaF(0.3);
    painter.setBrush(tColor);
    //绘制整个界面大小的阴影
    painter.drawRect(this->rect());
    painter.setBrush(Qt::NoBrush);
    QRect resultRect;
    //根据是否胜利以及是否有特效绘制不同位置不同大小的图片
    if (hasWin) {
        if (compositingStatus) {
            resultRect = QRect(320, 160, 388, 345);
        } else {
            resultRect = QRect(324, 160, 370, 331);
        }
    } else {
        if (compositingStatus) {
            resultRect = QRect(320, 160, 386, 345);
        } else {
            resultRect = QRect(326, 160, 372, 336);
        }
    }
    painter.drawPixmap(resultRect, backgroundImage);

    QWidget::paintEvent(event);
}

void Resultpopup::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

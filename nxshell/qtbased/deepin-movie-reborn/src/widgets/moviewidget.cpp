// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 *@file 这个文件是播放音乐时显示的窗口动画
 */
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include "compositing_manager.h"

#include "moviewidget.h"

#define DEFAULT_RATION (1.0f*1080/1920)     //背景图片比例
#define INTERVAL 50                         //刷新间隔
#define ROTATE_ANGLE 360/(1000*2.5/INTERVAL)  //2.5秒转一圈
#define DEFAULT_BGLENGTH 174                //背景边长
#define DEFAULT_NTLENGTH 48                 //音符边长

namespace dmr {
MovieWidget::MovieWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    initMember();

    if (!CompositingManager::get().composited()) {
        winId();
    }

    setAlignment(Qt::AlignCenter);
    setFrameShape(QFrame::Shape::NoFrame);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setMouseTracking(true);

    m_pScene = new QGraphicsScene;
    m_pScene->setBackgroundBrush(QBrush(QColor(0, 0, 0)));
    this->setScene(m_pScene);

    m_pBgRender = new QSvgRenderer(QString(":/resources/icons/music_bg.svg"));
    m_pNoteRender = new QSvgRenderer(QString(":/resources/icons/music_note.svg"));

    m_pBgSvgItem = new QGraphicsSvgItem;
    m_pNoteSvgItem = new QGraphicsSvgItem;
    m_pBgSvgItem->setSharedRenderer(m_pBgRender);
    m_pNoteSvgItem->setSharedRenderer(m_pNoteRender);
    m_pBgSvgItem->setCacheMode(QGraphicsItem::NoCache);
    m_pNoteSvgItem->setCacheMode(QGraphicsItem::NoCache);
    m_pScene->setSceneRect(m_pBgSvgItem->boundingRect());   //要在设置位置之前，不然动画会跳动
    m_pBgSvgItem->setPos((m_pScene->width() - DEFAULT_BGLENGTH) / 2, (m_pScene->height() - DEFAULT_BGLENGTH) / 2);
    m_pNoteSvgItem->setPos((m_pScene->width() - m_pNoteSvgItem->boundingRect().width()) / 2, (m_pScene->height() -  m_pNoteSvgItem->boundingRect().width())  / 2);

    m_pScene->addItem(m_pBgSvgItem);
    m_pScene->addItem(m_pNoteSvgItem);

    m_pTimer = new QTimer();
    m_pTimer->setInterval(INTERVAL);
    connect(m_pTimer, &QTimer::timeout, this, &MovieWidget::updateView);
}

MovieWidget::~MovieWidget()
{
    m_pTimer->deleteLater();
    m_pTimer = nullptr;
}

void MovieWidget::startPlaying()
{
    if (m_state == PlayState::STATE_STOP) {
        m_nRotate = 0;
        show();
    }

    m_pTimer->start();
    m_state = PlayState::STATE_PLAYING;
}

void MovieWidget::stopPlaying()
{
    m_pTimer->stop();
    m_state = PlayState::STATE_STOP;
    hide();
}

void MovieWidget::pausePlaying()
{
    m_pTimer->stop();
    m_state = PlayState::STATE_PAUSE;
}

void MovieWidget::updateView()
{
    qreal fRatio = 1.0;
    QRect rectDesktop;
    int nWidth = 0;
    int nHeight = 0;

    m_nRotate += ROTATE_ANGLE;

    nWidth = rect().width();
    nHeight = rect().height();
    rectDesktop = qApp->desktop()->availableGeometry(this);

    //根据比例缩放背景
    if (1.0f * nHeight / nWidth < DEFAULT_RATION) {
        nWidth = static_cast<int>(nHeight / DEFAULT_RATION);
        fRatio = nWidth * 2.0 / rectDesktop.width();
    } else {
        nHeight = static_cast<int>(nWidth * DEFAULT_RATION);
        fRatio = nHeight * 2.0 / rectDesktop.height();
    }

    m_pBgSvgItem->setScale(fRatio);
    m_pNoteSvgItem->setScale(fRatio);

    m_pBgSvgItem->setPos((m_pScene->width() - DEFAULT_BGLENGTH * fRatio) / 2, (m_pScene->height() - DEFAULT_BGLENGTH * fRatio) / 2);
    m_pNoteSvgItem->setPos((m_pScene->width() - m_pNoteSvgItem->boundingRect().width()) / 2, (m_pScene->height() -  m_pNoteSvgItem->boundingRect().width())  / 2);
    m_pNoteSvgItem->setTransformOriginPoint(24, 24);

    m_nRotate += ROTATE_ANGLE;
    m_pNoteSvgItem->setRotation(m_nRotate);
    viewport()->update();
}

void MovieWidget::initMember()
{
    m_nRotate = 0;
    m_state = PlayState::STATE_STOP;
    m_pTimer = nullptr;
    m_pBgSvgItem = nullptr;
    m_pNoteSvgItem = nullptr;
    m_pScene = nullptr;
    m_pBgRender = nullptr;
    m_pNoteRender = nullptr;
}

void MovieWidget::mousePressEvent(QMouseEvent *pEvent)
{
    pEvent->ignore();
}

void MovieWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    pEvent->ignore();
}

void MovieWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
     pEvent->ignore();
}

}

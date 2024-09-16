// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file
 * 此文件为切换播放暂停时窗口中间显示控件。
 *
 */
#include <QDateTime>
#include <QDebug>
#include <QEventLoop>
#include <QPainter>

#include "animationlabel.h"
#include "mainwindow.h"
#include "utils.h"

#define ANIMATION_TIME 250  ///动画时长
#define DELAY_TIME 2000 ///显示动画与隐藏动画间隔
using namespace dmr;
/**
 * @brief AnimationLabel构造函数
 * @param parent 父窗口
 * @param pMainWindow 主窗口
 */
AnimationLabel::AnimationLabel(QWidget *parent, QWidget *pMainWindow)
    : QFrame(parent)
{
    initMember(pMainWindow);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    resize(200, 200);
}

/**
 * @brief 由播放状态变为暂停状态
 */
void AnimationLabel::pauseAnimation()
{
    if (m_pPauseAnimationGroup && m_pPauseAnimationGroup->state() == QAbstractAnimation::Running)
        m_pPauseAnimationGroup->stop();
    m_pPlayAnimationGroup->start();
    if(!isVisible()) {
        show();
    }
}

/**
 * @brief 由暂停状态变为播放状态
 */
void AnimationLabel::playAnimation()
{
    if (m_pPlayAnimationGroup && m_pPlayAnimationGroup->state() == QAbstractAnimation::Running)
        m_pPlayAnimationGroup->stop();
    m_pPauseAnimationGroup->start();
    if(!isVisible()) {
        show();
    }
}

void AnimationLabel::setWM(bool isWM)
{
    m_bIsWM = isWM;
}

/**
 * @brief 初始化成员变量
 * @param mainwindow 主窗口指针
 */
void AnimationLabel::initMember(QWidget *pMainwindow)
{
    initPlayAnimation();
    initPauseAnimation();
    m_pMainWindow = pMainwindow;
    m_sFileName = "";
}

/**
 * @brief 初始化切换暂停时的动画组
 */
void AnimationLabel::initPauseAnimation()
{
    m_pPauseAnimationGroup = new QSequentialAnimationGroup(this);
    m_pPauseShowAnimation = new QPropertyAnimation(this, "fps");
    m_pPauseHideAnimation = new QPropertyAnimation(this, "fps");

    const int nShowAnimationNum = 9;  //显示动画图像数量
    const int nHideAnimationNum = 18; //隐藏动画图像数量
    m_pPauseShowAnimation->setDuration(ANIMATION_TIME);
    m_pPauseShowAnimation->setEasingCurve(QEasingCurve::InQuart);
    m_pPauseShowAnimation->setStartValue(0);
    m_pPauseShowAnimation->setEndValue(nShowAnimationNum);
    connect(m_pPauseShowAnimation, &QPropertyAnimation::valueChanged, this,
            &AnimationLabel::onPauseAnimationChanged);

    m_pPauseHideAnimation->setDuration(ANIMATION_TIME);
    m_pPauseHideAnimation->setEasingCurve(QEasingCurve::InQuart);
    m_pPauseHideAnimation->setStartValue(nShowAnimationNum);
    m_pPauseHideAnimation->setEndValue(nHideAnimationNum);
    connect(m_pPauseHideAnimation, &QPropertyAnimation::valueChanged, this,
            &AnimationLabel::onPauseAnimationChanged);

    m_pPauseAnimationGroup->addAnimation(m_pPauseShowAnimation);
    m_pPauseAnimationGroup->addPause(DELAY_TIME);
    m_pPauseAnimationGroup->addAnimation(m_pPauseHideAnimation);
}

/**
 * @brief 初始化切换播放时的动画组
 */
void AnimationLabel::initPlayAnimation()
{
    m_pPlayAnimationGroup = new QSequentialAnimationGroup(this);
    m_pPlayShowAnimation = new QPropertyAnimation(this, "fps");
    m_pPlayHideAnimation = new QPropertyAnimation(this, "fps");

    const int nShowAnimationNum = 9;  //显示动画图像数量
    const int nHideAnimationNum = 18; //隐藏动画图像数量
    m_pPlayShowAnimation->setDuration(ANIMATION_TIME);
    m_pPlayShowAnimation->setEasingCurve(QEasingCurve::InQuart);
    m_pPlayShowAnimation->setStartValue(0);
    m_pPlayShowAnimation->setEndValue(nShowAnimationNum);
    connect(m_pPlayShowAnimation, &QPropertyAnimation::valueChanged, this,
            &AnimationLabel::onPlayAnimationChanged);

    m_pPlayHideAnimation->setDuration(ANIMATION_TIME);
    m_pPlayHideAnimation->setEasingCurve(QEasingCurve::InQuart);
    m_pPlayHideAnimation->setStartValue(nShowAnimationNum);
    m_pPlayHideAnimation->setEndValue(nHideAnimationNum);
    connect(m_pPlayHideAnimation, &QPropertyAnimation::valueChanged, this,
            &AnimationLabel::onPlayAnimationChanged);

    m_pPlayAnimationGroup->addAnimation(m_pPlayShowAnimation);
    m_pPlayAnimationGroup->addPause(DELAY_TIME);
    m_pPlayAnimationGroup->addAnimation(m_pPlayHideAnimation);
}

/**
 * @brief 具体实现播放动画的每一帧图像显示
 * @param 当前显示图像的序号
 */
void AnimationLabel::onPlayAnimationChanged(const QVariant &value)
{
#if defined (__aarch64__) || defined (__mips__)
    if (m_bIsWM || utils::check_wayland_env()) {
        m_sFileName = QString(":/resources/icons/stop/%1.png").arg(value.toInt());
    } else {
        m_sFileName = QString(":/resources/icons/stop_new/%1.png").arg(value.toInt());
    }

#else
    m_sFileName = QString(":/resources/icons/stop/%1.png").arg(value.toInt());
#endif
    m_pixmap = QPixmap(m_sFileName);
    update();
}

/**
 * @brief 具体实现暂停动画的每一帧图像显示
 * @param 当前显示图像的序号
 */
void AnimationLabel::onPauseAnimationChanged(const QVariant &value)
{
#if defined (__aarch64__) || defined (__mips__)
    if (m_bIsWM || utils::check_wayland_env()) {
        m_sFileName = QString(":/resources/icons/start/%1.png").arg(value.toInt());
    } else {
        m_sFileName = QString(":/resources/icons/start_new/%1.png").arg(value.toInt());
    }

#else
    m_sFileName = QString(":/resources/icons/start/%1.png").arg(value.toInt());
#endif
    m_pixmap = QPixmap(m_sFileName);
    update();
}

void AnimationLabel::onHideAnimation()
{
    hide();
    if(m_pMainWindow) {
        m_pMainWindow->update();
    }
}

/**
 * @brief 重载绘制事件函数
 * @param event:qt绘制事件
 */
void AnimationLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawPixmap(rect(), m_pixmap);

    QFrame::paintEvent(e);
}

/**
 * @brief 重载显示事件函数
 * @param event:qt窗口显示事件
 */
void AnimationLabel::showEvent(QShowEvent *e)
{
    QFrame::showEvent(e);
}

/**
 * @brief 重载移动事件函数
 * @param event:qt窗口移动事件
 */
void AnimationLabel::moveEvent(QMoveEvent *e)
{
    return QFrame::moveEvent(e);
}


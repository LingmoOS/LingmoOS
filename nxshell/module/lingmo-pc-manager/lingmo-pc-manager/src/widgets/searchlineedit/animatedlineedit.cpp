// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "animatedlineedit.h"

#include "window/modules/common/compixmap.h"

#include <DGuiApplicationHelper>

#include <QDebug>
#include <QIcon>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QVariantAnimation>

DGUI_USE_NAMESPACE

#define GLOWING_IMG_WIDTH 100 // 动图宽（此设定值需要<=实际图片的宽）

AnimatedLineEdit::AnimatedLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_animation(nullptr)
{
    initUi();
    initConnection();
}

AnimatedLineEdit::~AnimatedLineEdit() { }

void AnimatedLineEdit::startAnimation()
{
    m_animation->stop();
    m_animation->start();
}

void AnimatedLineEdit::resizeEvent(QResizeEvent *e)
{
    updateIcon();
    QLineEdit::resizeEvent(e);
}

void AnimatedLineEdit::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    // Draw growing animation
    QPainter painter(this);
    int imgWidth = GLOWING_IMG_WIDTH;
    if (m_animation && m_animation->state() != QAbstractAnimation::Stopped) {
        float curValue = m_animation->currentValue().toFloat();
        // 计算动效水平偏移
        int xOffset = int((this->width() / 2 + imgWidth) * curValue);
        // 画向左动效
        float toLeftXPos = this->width() / 2 - xOffset;
        painter.drawPixmap(int(toLeftXPos), 0, m_toLeftGlowingImg);
        // 画向右动效
        float toRightXPos = this->width() / 2 - imgWidth + xOffset;
        painter.drawPixmap(int(toRightXPos), 0, m_toRightGlowingImg);
    }
}

void AnimatedLineEdit::updateIcon()
{
    QIcon icon = QIcon::fromTheme(ICON_ANIMATED_LINE_EDIT_GLOWING);
    if (icon.availableSizes().isEmpty()) {
        return;
    }
    m_toLeftGlowingImg = icon.pixmap(GLOWING_IMG_WIDTH, height());
    // 图片水平镜像
    QImage img = m_toLeftGlowingImg.toImage().mirrored(true, false);

    m_toRightGlowingImg = QPixmap::fromImage(img);
}

void AnimatedLineEdit::initUi()
{
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);

    // Animation
    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(616);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
    m_animation->setStartValue(QVariant(0.0f));
    m_animation->setEndValue(QVariant(1.0f));

    // 图片
    updateIcon();
}

void AnimatedLineEdit::initConnection()
{
    // 主题切换时
    connect(DGuiApplicationHelper::instance(),
            &DGuiApplicationHelper::themeTypeChanged,
            this,
            [this](DGuiApplicationHelper::ColorType type) {
                Q_UNUSED(type);
                this->updateIcon();
            });
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this] {
        this->update();
    });
}

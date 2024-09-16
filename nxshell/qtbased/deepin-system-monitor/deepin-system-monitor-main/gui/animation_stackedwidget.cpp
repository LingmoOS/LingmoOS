// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "animation_stackedwidget.h"

#include <QPainter>

AnimationStackedWidget::AnimationStackedWidget(const AnimationOri ori, QWidget *parent)
    : QStackedWidget(parent)
    , m_animationOri(ori)
    , m_Animation(new QPropertyAnimation(this, "offset"))
{
    setDuration(20);
    connect(m_Animation
            , &QPropertyAnimation::finished
            , this
            , &AnimationStackedWidget::animationFinished);
}

AnimationStackedWidget::~AnimationStackedWidget()
{
    if (m_Animation) {
        delete m_Animation;
        m_Animation = nullptr;
    }
}

void AnimationStackedWidget::setDuration(int duration)
{
    m_Duration = duration;
    m_Animation->setEasingCurve(QEasingCurve::InOutQuad);
}

void AnimationStackedWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (m_IsAnimation) {
        QPainter paint(this);
        paintCurrentWidget(paint, currentIndex());
        paintNextWidget(paint, m_NextIndex);
    }
}

void AnimationStackedWidget::animationFinished()
{
    m_IsAnimation = false;
    this->widget(currentIndex())->show();
    setCurrentIndex(m_NextIndex);
    emit signalIsFinished();
}

void AnimationStackedWidget::setCurrent(int index)
{
    //设置移动方向
    m_moveOri = getMoveOrientation(currentIndex(), index);
    setCurrentWidget(index, getBeginValue());
}

void AnimationStackedWidget::setCurrent(QWidget *widget)
{
    int index = this->indexOf(widget);
    if (index < 0)
        return;

    m_moveOri = getMoveOrientation(currentIndex(), index);
    setCurrentWidget(index, getBeginValue());
}

void AnimationStackedWidget::setCurrentWidget(int &index, int beginWidth)
{
    //如果正在动画，那么退出
    if (m_IsAnimation) {
        return;
    }
    //如果索引为当前索引则退出
    if (index == currentIndex()) {
        emit signalIsFinished();
        return;
    }
    m_IsAnimation = true;
    m_NextIndex = index;
    //隐藏当前的widget
    this->widget(currentIndex())->hide();
    m_Animation->setStartValue(beginWidth);
    m_Animation->setEndValue(0);
    m_Animation->setDuration(m_Duration);
    m_Animation->start();
}

int AnimationStackedWidget::getBeginValue()
{
    //存储窗口高度或宽度
    int value {0};
    //获取窗口矩阵
    QRect widgetRect = this->rect();
    switch (m_animationOri) {
    case LR: {
        value = widgetRect.width();
    }
    break;
    case TB: {
        value = widgetRect.height();
    }
    }
    return value;
}

AnimationStackedWidget::MoveOrientation AnimationStackedWidget::getMoveOrientation(const int currIndex, const int nextIndex)
{
    MoveOrientation moveOri{LeftToRight};
    if (nextIndex < currIndex) {
        switch (m_animationOri) {
        case LR: {
            moveOri = LeftToRight;
        }
        break;
        case TB: {
            moveOri = TopToBottom;
        }
        break;
        }
    } else if (nextIndex > currIndex) {
        switch (m_animationOri) {
        case LR: {
            moveOri = RightToLeft;
        }
        break;
        case TB: {
            moveOri = BottomToTop;
        }
        break;
        }
    }
    return moveOri;
}

void AnimationStackedWidget::paintCurrentWidget(QPainter &paint, int currentIndex)
{
    //获得当前页面的Widget
    QWidget *currentWidget = widget(currentIndex);
    QPixmap currentPixmap(currentWidget->size());

    //将当前窗口绘制到pixmap中
    currentWidget->render(&currentPixmap);
    QRect widgetRect = this->rect();
    //绘制当前的Widget
    double value = m_offset;
    //窗口宽度
    const int widgetWidth = widgetRect.width();
    //窗口高度
    const int widgetHeigth = widgetRect.height();
    QRectF targetRect;
    QRectF sourceRect;
    switch (m_moveOri) {
    case LeftToRight: {
        targetRect = QRectF(widgetWidth - value, 0, value, widgetHeigth);
        sourceRect = QRectF(0.0, 0.0, value, widgetHeigth);
    }
    break;
    case RightToLeft: {
        targetRect = QRectF(0.0, 0.0, value, widgetHeigth);
        sourceRect = QRectF(widgetWidth - value, 0, value, widgetHeigth);
    }
    break;
    case TopToBottom: {
        targetRect = QRectF(0.0, widgetHeigth - value, widgetWidth, value);
        sourceRect = QRectF(0.0, 0.0, widgetWidth, value);
    }
    break;
    case BottomToTop: {
        targetRect = QRectF(0.0, 0.0, widgetWidth, value);
        sourceRect = QRectF(0, widgetHeigth - value, widgetWidth, value);
    }
    break;
    }
    paint.drawPixmap(targetRect,
                     currentPixmap,
                     sourceRect);
}

void AnimationStackedWidget::paintNextWidget(QPainter &paint, int nextIndex)
{
    QWidget *nextWidget = widget(nextIndex);
    QRect widgetRect = this->rect();
    //设置下一个窗口的大小
    nextWidget->resize(widgetRect.width(), widgetRect.height());
    QPixmap nextPixmap(nextWidget->size());
    nextWidget->render(&nextPixmap);
    double value = m_offset;
    //窗口宽度
    const int widgetWidth = widgetRect.width();
    //窗口高度
    const int widgetHeigth = widgetRect.height();
    QRectF targetRect;
    QRectF sourceRect;
    switch (m_moveOri) {
    case LeftToRight: {
        targetRect = QRectF(0.0, 0.0, widgetWidth - value, widgetHeigth);
        sourceRect = QRectF(value, 0.0, widgetWidth - value, widgetHeigth);
    }
    break;
    case RightToLeft: {
        targetRect = QRectF(value, 0.0, widgetWidth - value, widgetHeigth);
        sourceRect = QRectF(0.0, 0.0, widgetWidth - value, widgetHeigth);
    }
    break;
    case TopToBottom: {
        targetRect = QRectF(0.0, 0.0, widgetWidth, widgetHeigth - value);
        sourceRect = QRectF(0.0, value,  widgetWidth, widgetHeigth - value);
    }
    break;
    case BottomToTop: {
        targetRect = QRectF(0.0, value,  widgetWidth, widgetHeigth - value);
        sourceRect = QRectF(0.0, 0.0, widgetWidth, widgetHeigth - value);
    }
    break;
    }
    paint.drawPixmap(targetRect,
                     nextPixmap,
                     sourceRect);
}

double AnimationStackedWidget::offset() const
{
    return m_offset;
}

void AnimationStackedWidget::setOffset(double offset)
{
    m_offset = offset;
    update();
}

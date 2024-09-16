// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "animationstackedwidget.h"

#include <QPainter>
#include <QMouseEvent>

AnimationStackedWidget::AnimationStackedWidget(const AnimationOri ori, QWidget *parent)
    : QStackedWidget(parent)
    , m_animationOri(ori)
    , m_Animation(new QPropertyAnimation(this, "offset", this))
{
    setDuration(1000);
    connect(m_Animation, &QPropertyAnimation::finished, this, &AnimationStackedWidget::animationFinished);
}

AnimationStackedWidget::~AnimationStackedWidget()
{
    delete m_Animation;
}

/**
 * @brief setDuration       设置动画持续时间
 * @param duration          动画时间
 */
void AnimationStackedWidget::setDuration(int duration)
{
    m_Duration = duration;
    m_Animation->setEasingCurve(QEasingCurve::InOutQuad);
}

/**
 * @brief paintEvent    窗口绘制事件
 * @param e             绘制事件
 */
void AnimationStackedWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (m_IsAnimation) {
        QPainter paint(this);
        paintCurrentWidget(paint, currentIndex());
        paintNextWidget(paint, m_NextIndex);
    }
}

void AnimationStackedWidget::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief animationFinished     动画切换完成
 */
void AnimationStackedWidget::animationFinished()
{
    m_IsAnimation = false;
    setCurrentIndex(m_NextIndex);
    //如果有需要跳转的页面
    if (m_nextGotoIndex != -1) {
        setCurrent(m_nextGotoIndex);
        //
        m_nextGotoIndex = -1;
        return;
    }
    emit signalIsFinished();
}

/**
 * @brief setCurrent        设置当前窗口
 * @param index             索引
 */
void AnimationStackedWidget::setCurrent(int index)
{
    //设置移动方向
    m_moveOri = getMoveOrientation(currentIndex(), index);
    setCurrentWidget(index, getBeginValue());
}

/**
 * @brief AnimationStackedWidget::setPre设置上一个索引的窗口
 */
void AnimationStackedWidget::setPre()
{
    //获取堆窗口数
    const int count = this->count();
    switch (m_animationOri) {
    case LR: {
        m_moveOri = LeftToRight;
    }
    break;
    case TB: {
        m_moveOri = TopToBottom;
    }
    break;
    }
    //获取下一个窗口编号
    int nextIndex = currentIndex() - 1;
    if (nextIndex < 0) {
        nextIndex = nextIndex + count;
    }
    setCurrentWidget(nextIndex, getBeginValue());
}

/**
 * @brief setNext       设置下一个索引的窗口
 */
void AnimationStackedWidget::setNext()
{
    //获取堆窗口数
    const int count = this->count();
    switch (m_animationOri) {
    case LR: {
        m_moveOri = RightToLeft;
    }
    break;
    case TB: {
        m_moveOri = BottomToTop;
    }
    break;
    }
    //获取下一个窗口编号
    int nextIndex = (currentIndex() + 1) % count;
    setCurrentWidget(nextIndex, getBeginValue());
}

/**
 * @brief setCurrentWidget      根据索引设置当前窗口，开启动画绘制
 * @param index                 索引
 * @param beginWidth            动画起始值
 */
void AnimationStackedWidget::setCurrentWidget(int &index, int beginWidth)
{
    //如果正在动画，那么退出
    if (m_IsAnimation) {
        m_nextGotoIndex = index;
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

/**
 * @brief getBeginValue         获取动画起始值
 * @return                      返回动画起始值
 */
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

/**
 * @brief getMoveOrientation    根据当前索引和下一个索引获取动画方向
 * @param currIndex             当前索引
 * @param nextIndex             设置的索引
 * @return                      动画绘制方向
 */
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

/**
 * @brief paintCurrentWidget        获取当前窗口的图片动态绘制
 * @param paint                     绘制对象
 * @param nextIndex                 窗口索引
 */
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
    const int widgetHeight = widgetRect.height();
    QRectF targetRect;
    QRectF sourceRect;
    switch (m_moveOri) {
    case LeftToRight: {
        targetRect = QRectF(widgetWidth - value, 0, value, widgetHeight);
        sourceRect = QRectF(0.0, 0.0, value, widgetHeight);
    }
    break;
    case RightToLeft: {
        targetRect = QRectF(0.0, 0.0, value, widgetHeight);
        sourceRect = QRectF(widgetWidth - value, 0, value, widgetHeight);
    }
    break;
    case TopToBottom: {
        targetRect = QRectF(0.0, widgetHeight - value, widgetWidth, value);
        sourceRect = QRectF(0.0, 0.0, widgetWidth, value);
    }
    break;
    case BottomToTop: {
        targetRect = QRectF(0.0, 0.0, widgetWidth, value);
        sourceRect = QRectF(0, widgetHeight - value, widgetWidth, value);
    }
    }
    paint.drawPixmap(targetRect, currentPixmap, sourceRect);
}

/**
 * @brief paintNextWidget   获取下一个窗口的图片动态绘制
 * @param paint         绘制对象
 * @param nextIndex     窗口索引
 */
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
    const int widgetHeight = widgetRect.height();
    QRectF targetRect;
    QRectF sourceRect;
    switch (m_moveOri) {
    case LeftToRight: {
        targetRect = QRectF(0.0, 0.0, widgetWidth - value, widgetHeight);
        sourceRect = QRectF(value, 0.0, widgetWidth - value, widgetHeight);
    }
    break;
    case RightToLeft: {
        targetRect = QRectF(value, 0.0, widgetWidth - value, widgetHeight);
        sourceRect = QRectF(0.0, 0.0, widgetWidth - value, widgetHeight);
    }
    break;
    case TopToBottom: {
        targetRect = QRectF(0.0, 0.0, widgetWidth, widgetHeight - value);
        sourceRect = QRectF(0.0, value, widgetWidth, widgetHeight - value);
    }
    break;
    case BottomToTop: {
        targetRect = QRectF(0.0, value, widgetWidth, widgetHeight - value);
        sourceRect = QRectF(0.0, 0.0, widgetWidth, widgetHeight - value);
    }
    }
    paint.drawPixmap(targetRect, nextPixmap, sourceRect);
}

/**
 * @brief offset    获取移动偏移量
 * @return          移动偏移量
 */
double AnimationStackedWidget::offset() const
{
    return m_offset;
}

/**
 * @brief setOffset         设置移动偏移量
 * @param offset            移动偏移量
 */
void AnimationStackedWidget::setOffset(double offset)
{
    m_offset = offset;
    update();
}

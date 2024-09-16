// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include <QDebug>

#include <QMoveEvent>
#include <QWidget>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QPropertyAnimation>
#include <QLabel>
#include <DWindowManagerHelper>
#include <QPointer>
DGUI_USE_NAMESPACE

WIDGETS_FRAME_BEGIN_NAMESPACE
DragDropFilter::DragDropFilter(QObject *parent)
    : QObject(parent)
{
}

bool DragDropFilter::eventFilter(QObject *obj, QEvent *event) {
    if (auto mouseEvent = dynamic_cast<QMouseEvent *>(event)) {
        if (mouseEvent->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonPress) {
            pressFlag = true;
        } else if (mouseEvent->type() == QEvent::MouseMove) {
            if (pressFlag) {
                pressFlag = false;
                auto pre = dynamic_cast<QMouseEvent *>(event);
                auto widget = qobject_cast<QWidget *>(obj);
                auto pos = widget->mapToParent(pre->localPos().toPoint());
                auto e = new QMouseEvent(QEvent::MouseMove, QPointF(pos.x(), pos.y()), pre->button(), pre->buttons(), pre->modifiers());
                qApp->postEvent(parent(), e);
                return true;
            }
        } else {
            pressFlag = false;
        }
    }
    return QObject::eventFilter(obj, event);
}

DragDropWidget::DragDropWidget(QWidget *parent)
    : QWidget (parent)
{
}

void DragDropWidget::gobackDrag(const QPixmap &pixmap, const QPoint &pos)
{
    qDebug() << "gobackDrag" << pos << m_startDrag;
    QLabel *widget = new QLabel();
    // avoid to being closed because `MainView` is a Tool.
    widget->setWindowFlag(Qt::SubWindow);
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    widget->setFixedSize(pixmap.size());
    widget->setPixmap(pixmap);
    widget->move(pos);
    widget->show();

    auto currentXAni = new QPropertyAnimation(widget, "pos");
    const int AnimationTime = 250;
    currentXAni->setEasingCurve(QEasingCurve::OutCubic);
    currentXAni->setDuration(AnimationTime);
    currentXAni->setStartValue(pos);
    currentXAni->setEndValue(m_startDrag);
    currentXAni->setDirection(QAbstractAnimation::Forward);
    currentXAni->start();
    connect(currentXAni, &QPropertyAnimation::finished, currentXAni, &QWidget::deleteLater);
    connect(currentXAni, &QPropertyAnimation::finished, widget, &QWidget::deleteLater);
}

void DragDropWidget::customEvent(QEvent *event)
{
    if (event->type() == LongPressDragEvent::Type) {
        auto le = dynamic_cast<LongPressDragEvent *>(event);
        qDebug() << "DragDropWidget() deal with LongPessDragEvent";

        startDrag(le->pos());
    }
}

AnimationWidgetItem::AnimationWidgetItem(QWidget *widget)
    : QWidgetItemV2(widget)
    , QObject(widget)
    , m_currentXAni (new QPropertyAnimation(this, "geometry"))
{
    const int AnimationTime = 300;
    m_currentXAni->setEasingCurve(QEasingCurve::Linear);
    m_currentXAni->setDirection(QAbstractAnimation::Forward);
    m_currentXAni->setDuration(AnimationTime);
    connect(m_currentXAni, &QPropertyAnimation::finished, this, &AnimationWidgetItem::moveFinished);
}

void AnimationWidgetItem::setGeometry(const QRect &rect)
{
    m_currentXAni->setStartValue(geometry());
    m_currentXAni->setEndValue(rect);
    m_currentXAni->start();
}

void AnimationWidgetItem::updateGeometry(const QRect &rect)
{
    QWidgetItemV2::setGeometry(rect);
}

bool hasComposite()
{
    return DWindowManagerHelper::instance()->hasComposite();
}

bool releaseMode()
{
    static bool on = qEnvironmentVariableIsSet("DDE_WIDGETS_DBUG_MODE");
    if (on)
        return qEnvironmentVariableIntValue("DDE_WIDGETS_DBUG_MODE") <= 0;

#if (defined QT_DEBUG)
    return false;
#endif
    return true;
}

int LongPressDragEvent::Type = QEvent::registerEventType(QEvent::User + 10);
LongPressDragEvent::LongPressDragEvent(const QMouseEvent &me)
    : QMouseEvent(static_cast<QEvent::Type>(Type), me.localPos(), me.button(), me.buttons(), me.modifiers())
{

}

LongPressEventFilter::LongPressEventFilter(QWidget *parent)
    : QObject (parent)
{
}

QWidget *LongPressEventFilter::targetWidget(const QPoint &global) const
{
    // get widget at global screen position point instead of children order point.
    auto target = QApplication::widgetAt(global);
    while (target && target->isWidgetType()) {
        if ((target->windowType() & Qt::SubWindow) == Qt::SubWindow
                || (target->windowType() & Qt::Dialog) == Qt::Dialog
                || (target->windowType() & Qt::Popup) == Qt::Popup)
            return nullptr;
        // get correct object by class type, customEvent can't be distributed like MouseEvent
        // when using sendEvent, it's not passed to parent class when not processed.
        if (qobject_cast<DragDropWidget *>(target))
            return target;

        target = target->parentWidget();
    }
    return nullptr;
}

bool LongPressEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto me = dynamic_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            // only pressed on targetWidget to check LongPress event.
            if (!targetWidget(me->globalPos()))
                break;

            m_longPressTimer.start(m_longPressInterval, this);

            // store the presseEvent to restore lower widget's status if it's a long press.
            m_leftPressInfo.obj = obj;
            m_leftPressInfo.local = me->localPos();
            m_leftPressInfo.button = me->button();
            m_leftPressInfo.buttons = me->buttons();
            m_leftPressInfo.modifiers = me->modifiers();
        }
    } break;
    case QEvent::MouseMove: {
        if (m_isLongPress) {
            auto me = dynamic_cast<QMouseEvent *>(event);
            if (auto target = targetWidget(me->globalPos())) {
                qDebug() << "LongPressEventFilter() send LongPressDrag" << target;
                auto pos = target->mapFromGlobal(me->globalPos());
                LongPressDragEvent le(*me);
                le.setLocalPos(pos);
                qApp->sendEvent(target, &le);
            }
        }
        resetLongPressStatus();
    } break;
    case QEvent::MouseButtonRelease: {
        resetLongPressStatus();
    } break;
    }

    return QObject::eventFilter(obj, event);
}

void LongPressEventFilter::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_longPressTimer.timerId()) {
        if (m_leftPressInfo.obj) {
            QMouseEvent e(QEvent::MouseButtonRelease,
                          m_leftPressInfo.local,
                          m_leftPressInfo.button,
                          m_leftPressInfo.buttons,
                          m_leftPressInfo.modifiers);

            // blockSignals avoid to emitting composition signal of press and release.
            // emit release event to cancle MouseButtonPress.

            const auto obj = m_leftPressInfo.obj;
            obj->blockSignals(true);
            qApp->sendEvent(obj, &e);
            obj->blockSignals(false);
        }
        resetLongPressStatus();
        m_isLongPress = true;
    }
}

void LongPressEventFilter::resetLongPressStatus()
{
    m_longPressTimer.stop();
    m_isLongPress = false;
    m_leftPressInfo.obj = nullptr;
}
WIDGETS_FRAME_END_NAMESPACE

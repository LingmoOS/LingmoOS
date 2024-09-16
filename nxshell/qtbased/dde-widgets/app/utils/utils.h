// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include <QObject>
#include <QEvent>
#include <QWidget>
#include <QLayoutItem>
#include <QPropertyAnimation>
#include <QBasicTimer>
#include <QMouseEvent>
#include <QPointer>

WIDGETS_FRAME_BEGIN_NAMESPACE
class DragDropFilter : public QObject {
    Q_OBJECT
public:
    explicit DragDropFilter(QObject *parent);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    bool pressFlag = false;
};

class DragDropWidget : public QWidget {
    Q_OBJECT
public:
    explicit DragDropWidget(QWidget *parent = nullptr);

protected:
    virtual void customEvent(QEvent *event) override;

    virtual void startDrag(const QPoint &pos) { Q_UNUSED(pos);}
    virtual void gobackDrag(const QPixmap &pixmap, const QPoint &pos);

protected:
    QPoint m_startDrag;
};
class AnimationWidgetItem : public QObject, public QWidgetItemV2 {
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE updateGeometry)
public:
    explicit AnimationWidgetItem(QWidget *widget);
    virtual void setGeometry(const QRect &rect) override;

Q_SIGNALS:
    void moveFinished();
public Q_SLOTS:
    void updateGeometry(const QRect &rect);

private:
    QPropertyAnimation *m_currentXAni = nullptr;
};

bool hasComposite();

bool releaseMode();

class LongPressDragEvent : public QMouseEvent {
public:
    explicit LongPressDragEvent(const QMouseEvent &me);

    static int Type;
};

class LongPressEventFilter : public QObject {
    Q_OBJECT
public:
    explicit LongPressEventFilter(QWidget *parent = nullptr);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;
    QWidget *targetWidget(const QPoint &global) const;

private:
    void resetLongPressStatus();

private:
    bool m_isLongPress = false;
    QBasicTimer m_longPressTimer;
    int m_longPressInterval = 300;
    struct LeftPressInfo {
        QPointer<QObject> obj = nullptr;
        QPointF local;
        Qt::MouseButton button;
        Qt::MouseButtons buttons;
        Qt::KeyboardModifiers modifiers;
    };
    LeftPressInfo m_leftPressInfo;
};
WIDGETS_FRAME_END_NAMESPACE

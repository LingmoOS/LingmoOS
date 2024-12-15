// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filter.h"

#include <DWidget>
#include <DUtil>

#include <QDebug>
#include <QTimer>
#include <QEvent>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsDropShadowEffect>
#include <QApplication>


HoverFilter::HoverFilter(QObject *parent) : QObject(parent)
{

}

bool HoverFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        auto w = qobject_cast<QWidget *>(obj);
        w->setCursor(QCursor(Qt::PointingHandCursor));
//        qDebug() << "set cursor" << w << w->cursor();
//        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        return QObject::eventFilter(obj, event);
    }
    case QEvent::Leave: {
        auto w = qobject_cast<QWidget *>(obj);
//        qDebug() << "unset cursor" << w;
        w->unsetCursor();
        QApplication::restoreOverrideCursor();
        return QObject::eventFilter(obj, event);
    }
    default:
        return QObject::eventFilter(obj, event);
    }
}


//class HintFilterPrivate
//{
//public:
//    explicit HintFilterPrivate(HintFilter *parent) : q_ptr(parent) {}

//    void showHint(QWidget *hint);

//    QTimer  *delayShowTimer = nullptr;

//    QWidget *parentWidget = nullptr;
//    QWidget *hintWidget = nullptr;

//    HintFilter *q_ptr;
//    Q_DECLARE_PUBLIC(HintFilter)
//};


void HintFilterPrivate::showHint(QWidget *hint)
{
    if (!parentWidget) {
        return;
    }
    auto w = parentWidget;
    if (hintWidget && hintWidget != hint) {
        hintWidget->hide();
    }
    hintWidget = hint;
    if (!hintWidget) {
        return;
    }


    DUtil::TimerSingleShot(10, [w, this]() {
        auto centerPos = w->mapToGlobal(w->rect().center());

        hintWidget->show();
        hintWidget->adjustSize();

        auto sz = hintWidget->size();
        centerPos.setX(centerPos.x()  - sz.width() / 2 + 2);
        centerPos.setY(centerPos.y() - 32 - sz.height());
        if (centerPos.y() - 32 - sz.height() <= 0) {
            centerPos.setY(centerPos.y() + w->height() + 22 + sz.height());
        }
        centerPos = hintWidget->mapFromGlobal(centerPos);
        centerPos = hintWidget->mapToParent(centerPos);
        hintWidget->move(centerPos);
        hintWidget->raise();
    });
}

HintFilter::HintFilter(QObject *parent)  : QObject(parent), d_ptr(new HintFilterPrivate(this))
{
    Q_D(HintFilter);
    d->delayShowTimer = new QTimer(this);
    d->delayShowTimer->setInterval(1000);
    connect(d->delayShowTimer, &QTimer::timeout, this, [ = ]() {
        if (d->parentWidget) {
            auto hint = d->parentWidget->property("HintWidget").value<QWidget *>();
            d->showHint(hint);
        }
        d->delayShowTimer->stop();
    });
}

HintFilter::~HintFilter()
{

}
/*
void HintFilter::hideAll()
{
    Q_D(HintFilter);
    if (d->hintWidget) {
        d->hintWidget->hide();
        d->delayShowTimer->stop();
    }
}
*/
bool HintFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(HintFilter);
    switch (event->type()) {
    case QEvent::Enter: {
        if (d->hintWidget) {
            d->hintWidget->hide();
        }

        auto w = qobject_cast<QWidget *>(obj);
        d->parentWidget = w;
        if (!w) {
            break;
        }

        d->hintWidget = w->property("HintWidget").value<QWidget *>();
        if (!d->hintWidget) {
            break;
        }

        d->delayShowTimer->stop();

        bool nodelayshow = d->hintWidget->property("NoDelayShow").toBool();
        if (nodelayshow) {
            d->showHint(d->hintWidget);
        } else {
            d->delayShowTimer->start();
        }

        d->parentWidget->setCursor(QCursor(Qt::PointingHandCursor));
        break;
    }
    case QEvent::Leave: {
        if (d->hintWidget) {
            if (!d->hintWidget->property("DelayHide").toBool()) {
                d->hintWidget->hide();
                d->delayShowTimer->stop();
            } else {
                QMetaObject::invokeMethod(d->hintWidget, "deleyHide", Qt::DirectConnection);
            }

        }
        auto w = qobject_cast<QWidget *>(obj);
        if (w) {
            w->unsetCursor();
        }

        break;
    }
    case QEvent::MouseButtonPress:
        if (d->hintWidget) {
            if (!d->hintWidget->property("_dm_keep_on_click").toBool()) {
                d->hintWidget->hide();
            }
            d->delayShowTimer->stop();
        }

        break;
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}
/*
void HintFilter::showHitsFor(QWidget *w, QWidget *hint)
{
    Q_D(HintFilter);
    if (d->hintWidget) {
        d->hintWidget->hide();
    }

    d->parentWidget = w;
    if (!w) {
        return;
    }

    d->hintWidget = hint;
    if (!d->hintWidget) {
        return;
    }

    d->delayShowTimer->stop();

    d->showHint(hint);

    d->hintWidget->setCursor(QCursor(Qt::PointingHandCursor));
}
*/
HoverShadowFilter::HoverShadowFilter(QObject *parent): QObject(parent)
{

}

bool HoverShadowFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        auto w = qobject_cast<QWidget *>(obj);
        auto shadow = new QGraphicsDropShadowEffect(w);
        shadow->setBlurRadius(8);
        shadow->setOffset(0, 0);
        shadow->setColor(Qt::white);
        w->setGraphicsEffect(shadow);
        w->setCursor(QCursor(Qt::PointingHandCursor));
        return QObject::eventFilter(obj, event);
    }
    case QEvent::Leave: {
        auto w = qobject_cast<QWidget *>(obj);
        w->graphicsEffect()->deleteLater();
        w->setGraphicsEffect(nullptr);
        w->unsetCursor();
        return QObject::eventFilter(obj, event);
    }
    default:
        return QObject::eventFilter(obj, event);
    }
}

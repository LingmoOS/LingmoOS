/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Tim D. Gilman <tdgilman@best.org>
    SPDX-FileCopyrightText: 1998-2001 Mirko Boehm <mirko@kde.org>
    SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpopupframe.h"

#include <QEventLoop>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QScreen>

class KPopupFramePrivate
{
public:
    KPopupFramePrivate(KPopupFrame *qq);
    ~KPopupFramePrivate();

    KPopupFrame *q;

    /**
     * The result. It is returned from exec() when the popup window closes.
     */
    int result;

    /**
     * The only subwidget that uses the whole dialog window.
     */
    QWidget *main;

    // TODO KF6: Remove this, add a hideEvent() reimplementation instead.
    class OutsideClickCatcher;
    OutsideClickCatcher *outsideClickCatcher;
};

class KPopupFramePrivate::OutsideClickCatcher : public QObject
{
    Q_OBJECT
public:
    explicit OutsideClickCatcher(QObject *parent = nullptr)
        : QObject(parent)
        , m_popup(nullptr)
    {
    }
    ~OutsideClickCatcher() override
    {
    }

    void setPopupFrame(KPopupFrame *popup)
    {
        m_popup = popup;
        popup->installEventFilter(this);
    }

    KPopupFrame *m_popup;

    bool eventFilter(QObject *object, QEvent *event) override
    {
        Q_UNUSED(object);

        // To catch outside clicks, it is sufficient to check for
        // hide events on Qt::Popup type widgets
        if (event->type() == QEvent::Hide && m_popup) {
            // do not set d->result here, because the popup
            // hides itself after leaving the event loop.
            Q_EMIT m_popup->leaveModality();
        }
        return false;
    }
};

KPopupFramePrivate::KPopupFramePrivate(KPopupFrame *qq)
    : q(qq)
    , result(0)
    , // rejected
    main(nullptr)
    , outsideClickCatcher(new OutsideClickCatcher)
{
    outsideClickCatcher->setPopupFrame(q);
}

KPopupFramePrivate::~KPopupFramePrivate()
{
    delete outsideClickCatcher;
}

KPopupFrame::KPopupFrame(QWidget *parent)
    : QFrame(parent, Qt::Popup)
    , d(new KPopupFramePrivate(this))
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setMidLineWidth(2);
}

KPopupFrame::~KPopupFrame() = default;

void KPopupFrame::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        d->result = 0; // rejected
        Q_EMIT leaveModality();
        // qApp->exit_loop();
    }
}

void KPopupFrame::hideEvent(QHideEvent *e)
{
    QFrame::hideEvent(e);
}

void KPopupFrame::close(int r)
{
    d->result = r;
    Q_EMIT leaveModality();
    // qApp->exit_loop();
}

void KPopupFrame::setMainWidget(QWidget *m)
{
    d->main = m;
    if (d->main) {
        resize(d->main->width() + 2 * frameWidth(), d->main->height() + 2 * frameWidth());
    }
}

void KPopupFrame::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    if (d->main) {
        d->main->setGeometry(frameWidth(),
                             frameWidth(), //
                             width() - 2 * frameWidth(), //
                             height() - 2 * frameWidth());
    }
}

void KPopupFrame::popup(const QPoint &pos)
{
    // Make sure the whole popup is visible.
    QScreen *screen = QGuiApplication::screenAt(pos);

    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();
    if (screen) {
        const QRect desktopGeometry = screen->geometry();
        if (x + w > desktopGeometry.x() + desktopGeometry.width()) {
            x = desktopGeometry.width() - w;
        }
        if (y + h > desktopGeometry.y() + desktopGeometry.height()) {
            y = desktopGeometry.height() - h;
        }
        if (x < desktopGeometry.x()) {
            x = 0;
        }
        if (y < desktopGeometry.y()) {
            y = 0;
        }
    }

    // Pop the thingy up.
    move(x, y);
    show();
    d->main->setFocus();
}

int KPopupFrame::exec(const QPoint &pos)
{
    popup(pos);
    repaint();
    d->result = 0; // rejected
    QEventLoop eventLoop;
    connect(this, &KPopupFrame::leaveModality, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    hide();
    return d->result;
}

int KPopupFrame::exec(int x, int y)
{
    return exec(QPoint(x, y));
}

#include "kpopupframe.moc"
#include "moc_kpopupframe.cpp"

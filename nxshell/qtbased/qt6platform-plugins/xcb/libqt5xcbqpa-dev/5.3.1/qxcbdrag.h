// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBDRAG_H
#define QXCBDRAG_H

#include <qpa/qplatformdrag.h>
#include <private/qsimpledrag_p.h>
#include <qxcbobject.h>
#include <xcb/xcb.h>
#include <qlist.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsharedpointer.h>
#include <qpointer.h>
#include <qvector.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <qbackingstore.h>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DRAGANDDROP

class QMouseEvent;
class QWindow;
class QXcbConnection;
class QXcbWindow;
class QXcbDropData;
class QXcbScreen;
class QDrag;
class QShapedPixmapWindow;

class QXcbDrag : public QXcbObject, public QBasicDrag
{
public:
    QXcbDrag(QXcbConnection *c);
    ~QXcbDrag();

    virtual QMimeData *platformDropData();


    void startDrag();
    void cancel();
    void move(const QMouseEvent *me);
    void drop(const QMouseEvent *me);
    void endDrag();

    void handleEnter(QWindow *window, const xcb_client_message_event_t *event);
    void handlePosition(QWindow *w, const xcb_client_message_event_t *event);
    void handleLeave(QWindow *w, const xcb_client_message_event_t *event);
    void handleDrop(QWindow *, const xcb_client_message_event_t *event);

    void handleStatus(const xcb_client_message_event_t *event);
    void handleSelectionRequest(const xcb_selection_request_event_t *event);
    void handleFinished(const xcb_client_message_event_t *event);

    bool dndEnable(QXcbWindow *win, bool on);

    void updatePixmap();
    xcb_timestamp_t targetTime() { return target_time; }

protected:
    void timerEvent(QTimerEvent* e);

private:
    friend class QXcbDropData;

    void init();

    void handle_xdnd_position(QWindow *w, const xcb_client_message_event_t *event);
    void handle_xdnd_status(const xcb_client_message_event_t *event);
    void send_leave();

    Qt::DropAction toDropAction(xcb_atom_t atom) const;
    xcb_atom_t toXdndAction(Qt::DropAction a) const;

    QPointer<QWindow> currentWindow;
    QPoint currentPosition;

    QXcbDropData *dropData;
    Qt::DropAction accepted_drop_action;

    QWindow *desktop_proxy;

    xcb_atom_t xdnd_dragsource;

    // the types in this drop. 100 is no good, but at least it's big.
    enum { xdnd_max_type = 100 };
    QList<xcb_atom_t> xdnd_types;

    // timestamp from XdndPosition and XdndDroptime for retrieving the data
    xcb_timestamp_t target_time;
    xcb_timestamp_t source_time;

    // rectangle in which the answer will be the same
    QRect source_sameanswer;
    bool waiting_for_status;

    // top-level window we sent position to last.
    xcb_window_t current_target;
    // window to send events to (always valid if current_target)
    xcb_window_t current_proxy_target;

    QXcbScreen *current_screen;

    // 10 minute timer used to discard old XdndDrop transactions
    enum { XdndDropTransactionTimeout = 600000 };
    int cleanup_timer;

    QVector<xcb_atom_t> drag_types;

    struct Transaction
    {
        xcb_timestamp_t timestamp;
        xcb_window_t target;
        xcb_window_t proxy_target;
        QWindow *targetWindow;
//        QWidget *embedding_widget;
        QPointer<QDrag> drag;
        QTime time;
    };
    QList<Transaction> transactions;

    int transaction_expiry_timer;
    void restartDropExpiryTimer();
    int findTransactionByWindow(xcb_window_t window);
    int findTransactionByTime(xcb_timestamp_t timestamp);
    xcb_window_t findRealWindow(const QPoint & pos, xcb_window_t w, int md, bool ignoreNonXdndAwareWindows);
};

#endif // QT_NO_DRAGANDDROP

QT_END_NAMESPACE

#endif

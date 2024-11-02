/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "window-manager.h"

#include <QWidget>
#include <QMouseEvent>

#include <KWindowSystem>
#include <NETWM>
#include <QX11Info>
#include <xcb/xcb.h>

#include <QApplication>
#include <QWindow>

#include <QDebug>
#include "settings/black-list.h"

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(100);
    m_timer.setSingleShot(true);
    m_start_point = QPoint(0, 0);

    qApp->installEventFilter(new AppEventFilter(this));

    if (QX11Info::isPlatformX11())
        return;

    using namespace KWayland::Client;
    m_connection = ConnectionThread::fromApplication(qApp);
    if (!m_connection)
        return;
    m_registry = new Registry(this);
    m_registry->create(m_connection);

    connect(m_registry, &KWayland::Client::Registry::seatAnnounced, this, [=](quint32 name, quint32 version) {
        m_seat = m_registry->createSeat(name, version, this);
        connect(m_seat, &Seat::hasPointerChanged, this, [=](bool pointerChanged) {
            if (pointerChanged) {
                if (!m_pointer) {
                    m_pointer = m_seat->createPointer(this);
                    connect(m_pointer,
                            &KWayland::Client::Pointer::buttonStateChanged,
                            this,
                            [this](quint32 serial, quint32 time, quint32 button, Pointer::ButtonState state) {
                                m_serial = serial;
                            });
                }
            } else {
                delete m_pointer;
                m_pointer = nullptr;
            }
        });
    });

    m_registry->setup();
    m_connection->roundtrip();
}

WindowManager::~WindowManager()
{
    if(m_registry){
        m_registry->deleteLater();
        m_registry = nullptr;
    }
}

void WindowManager::registerWidget(QWidget *w)
{
    if (dragable = isDragable(w)) {
        w->removeEventFilter(this);
        w->installEventFilter(this);
    }
}

void WindowManager::unregisterWidget(QWidget *w)
{
    if (dragable)
        w->removeEventFilter(this);
}

bool WindowManager::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent *event = static_cast<QMouseEvent*>(e);

        if (event->button() == Qt::LeftButton) {
            // If the cursor is not normal arrow cursor,
            // we should consider there is another excepted operation
            // which current widget hope do. So we won't trigger
            // the window move event.
            QWidget *w = qobject_cast<QWidget *>(obj);
            if (w->cursor().shape() != Qt::CursorShape::ArrowCursor)
                return false;

            buttonPresseEvent(obj, event);
            return false;
        }
        return false;
    }
    case QEvent::MouseMove: {
        //if (QWidget::mouseGrabber()) return false;
        //fixbug#182587
        if(windowManageBlackList().contains(qApp->applicationName()) || windowManageBlackList().contains(qAppName())){
            return false;
        }
        QMouseEvent *event = static_cast<QMouseEvent*>(e);

        /*! \note In x11, accuracy of the MouseEvent translated from TouchEvent is not
         *  consistent with the accuracy of the native MouseEvent(TouchEvent is a decimal
         *  and MouseEvent is an integer).
         *  When the touch is pressed, if the mouse clicks the right button, Qt will think that
         *  the positions of the two clicks are inconsistent, and will send a MouseMoveEvent,
         *  the source of this MouseMoveEvent is not MouseEventSynthesizedByQt, so this MouseMoveEvent
         *  will not tirgger w->grabMouse(), window will be grabbed by kwin but no TouchEndEvent is received
         *  Therefore, the MouseMoveEvent is filtered out here if the moving distance is too small.
         *  \return return true because some borderless windows handle window dragging separately,
         *  if return false will cause these windows to behave abnormally
         *  \author wangweinan@kylinos.cn
        */
        if (qAbs(event->pos().x() - m_start_point.x()) < 2 &&
            qAbs(event->pos().y() - m_start_point.y()) < 2) {
            return true;
        }

        //move request
        mouseMoveEvent(obj, event);
        return false;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        mouseReleaseEvent(obj, event);
        return false;
    }
    case QEvent::Move: {
        if (m_current_obj && m_is_dragging) {
            //qDebug()<<"drag end";
            endDrag();
        } else {
            //qDebug()<<"move";
        }
        return false;
    }
    default:
        return false;
    }
}

void WindowManager::buttonPresseEvent(QObject *obj, QMouseEvent *e)
{
    //qDebug()<<"mouse press event";
    endDrag();
    QWidget *w = qobject_cast<QWidget*>(obj);
    //NOTE: We have to skip the border for resize event.
    auto pos = w->mapFromGlobal(e->globalPos());

    //title active height set 48px
    QRect activeRect(0, 0, w->width(), 48);
    if (!activeRect.contains(pos)) {
        //qDebug()<<"skip move event";
        return;
    }
//    if (!w->rect().adjusted(10, 10, -10, -10).contains(pos)) {
//        //qDebug()<<"skip move event";
//        return;
//    }


    m_prepared_to_drag = true;
    m_is_dragging = false;
    m_current_obj = obj;
    m_start_point = e->pos();
    m_timer.start();
}

void WindowManager::mouseMoveEvent(QObject *obj, QMouseEvent *e)
{
    //qDebug()<<"mouse move event";

    if (!m_prepared_to_drag)
        return;

    if (m_is_dragging)
        return;

    QWidget *w = qobject_cast<QWidget*>(obj);
    const QPoint native = e->globalPos();
    qreal  dpiRatio = qApp->devicePixelRatio();
    if (QX11Info::isPlatformX11()) {
//        qDebug()<<"x11 move start";
        auto connection = QX11Info::connection();
        xcb_ungrab_pointer(connection, XCB_TIME_CURRENT_TIME);
        NETRootInfo(connection, NET::WMMoveResize).moveResizeRequest(w->winId(), native.x() * dpiRatio, native.y() * dpiRatio, NET::Move);
//        qDebug()<<"x11 move end";

        if (e->source() == Qt::MouseEventSynthesizedByQt) {
            if (!w->mouseGrabber()) {
                //! \note Under XI, grabMouse will grab touch devices at the same time.
                //! When the touch device grabbing changes, xserver will send TouchEndEvent
                //! to the client that monitors the touch sequence.
                //! \see qtbase QXcbConnection::xi2SetMouseGrabEnabled
                w->grabMouse();
                w->releaseMouse();
            }
        }

        xcb_button_release_event_t* event = new xcb_button_release_event_t;
        memset(event, 0x00, sizeof(xcb_button_release_event_t));
        event->response_type = XCB_BUTTON_RELEASE;
        event->event = w->winId();
        event->time = QX11Info::getTimestamp();
        event->same_screen = 1;
        event->root = QX11Info::appRootWindow();
        event->root_x = native.x();
        event->root_y = native.y();
        event->event_x = 0;
        event->event_y = 0;
        event->child = 0;
        event->state = 0;
        event->detail = XCB_BUTTON_INDEX_1;

        xcb_send_event(connection, false, w->winId(), XCB_EVENT_MASK_BUTTON_RELEASE, (char *) event);
        delete event;

        xcb_flush(connection);

        m_is_dragging = true;

        m_timer.start();
    } else {
        auto widget = qobject_cast<QWidget *>(obj);
        auto topLevel = widget->topLevelWidget();

#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
        topLevel->windowHandle()->startSystemMove();
#else
        auto shellSurface = KWayland::Client::ShellSurface::fromWindow(topLevel->windowHandle());
        if (!shellSurface)
            return;
        shellSurface->requestMove(m_seat, m_serial);
#endif
        m_is_dragging = true;
    }

    return;
}

void WindowManager::mouseReleaseEvent(QObject *obj, QMouseEvent *e)
{
    //qDebug()<<"mouse release event";
    endDrag();
}

void WindowManager::endDrag()
{
    m_is_dragging = false;
    m_current_obj = nullptr;
    m_start_point = QPoint(0, 0);
    m_timer.stop();
}

bool WindowManager::isDragable(QWidget *widget)
{
    if (widget && widget->isWindow()) {
        if ((widget->windowFlags() & Qt::Popup) == Qt::Popup) {
            if ((widget->windowFlags() & Qt::Tool) == Qt::Tool)
                return true;
            return false;
        }
        return true;
    } else {
        return false;
    }
}



// AppEventFilter
AppEventFilter::AppEventFilter(WindowManager *parent) : QObject(parent)
{
    m_wm = parent;
}

bool AppEventFilter::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonRelease) {
        m_wm->m_prepared_to_drag = false;
        m_wm->m_is_dragging = false;
        m_wm->m_current_obj = nullptr;
        m_wm->m_timer.stop();
        m_wm->m_start_point = QPoint();
    }

//    if (m_wm->m_is_dragging && m_wm->m_current_obj && (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseMove)) {
//        QMouseEvent mouseEvent(QEvent::MouseButtonRelease,
//                               m_wm->m_start_point,
//                               Qt::LeftButton,
//                               Qt::LeftButton,
//                               Qt::NoModifier);
//        qApp->sendEvent(m_wm->m_current_obj, &mouseEvent);
//    }
    return false;
}

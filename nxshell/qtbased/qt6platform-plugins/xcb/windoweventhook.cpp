// SPDX-FileCopyrightText: 2016 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// Fix compilation issue with multi-herited QXcbWindow since 6.6.2
#include <QDebug>
QT_BEGIN_NAMESPACE
class QXcbWindow;
class QPlatformWindow;
inline QDebug operator<<(QDebug dbg, const QXcbWindow *window) {
  return dbg << (const QPlatformWindow*)(window);
}
QT_END_NAMESPACE

#include "windoweventhook.h"
#include "vtablehook.h"
#include "utility.h"
#include "dframewindow.h"
#include "dplatformwindowhelper.h"
#include "dhighdpi.h"

#define private public
#define protected public
#include "qxcbdrag.h"
#include "qxcbkeyboard.h"
#undef private
#undef protected

#include <QDrag>
#include <QMimeData>

#include <private/qguiapplication_p.h>
#include <private/qwindow_p.h>

#include <X11/extensions/XI2proto.h>

DPP_BEGIN_NAMESPACE

PUBLIC_CLASS(QXcbWindow, WindowEventHook);
PUBLIC_CLASS(QDropEvent, WindowEventHook);

void WindowEventHook::init(QXcbWindow *window, bool redirectContent)
{
    const Qt::WindowType &type = window->window()->type();

    if (redirectContent) {
        VtableHook::overrideVfptrFun(window, &QXcbWindow::handleMapNotifyEvent,
                                     &WindowEventHook::handleMapNotifyEvent);
    }

    VtableHook::overrideVfptrFun(window, &QXcbWindow::handleConfigureNotifyEvent,
                                 &WindowEventHook::handleConfigureNotifyEvent);

    if (type == Qt::Widget || type == Qt::Window || type == Qt::Dialog) {
        VtableHook::overrideVfptrFun(window, &QXcbWindow::handleClientMessageEvent,
                                     &WindowEventHook::handleClientMessageEvent);
        VtableHook::overrideVfptrFun(window, &QXcbWindow::handleFocusInEvent,
                                     &WindowEventHook::handleFocusInEvent);
        VtableHook::overrideVfptrFun(window, &QXcbWindow::handleFocusOutEvent,
                                     &WindowEventHook::handleFocusOutEvent);
#ifdef XCB_USE_XINPUT22
        VtableHook::overrideVfptrFun(window, &QXcbWindow::handleXIEnterLeave,
                                     &WindowEventHook::handleXIEnterLeave);
#endif
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
        VtableHook::overrideVfptrFun(window, &QPlatformWindow::windowEvent,
                                     &WindowEventHook::windowEvent);
#else
        VtableHook::overrideVfptrFun(window, &QXcbWindow::windowEvent,
                                     &WindowEventHook::windowEvent);
#endif
    }

    if (type == Qt::Window) {
        VtableHook::overrideVfptrFun(window, &QXcbWindowEventListener::handlePropertyNotifyEvent,
                                     &WindowEventHook::handlePropertyNotifyEvent);
    }
}

//#define DND_DEBUG
#ifdef DND_DEBUG
#define DEBUG qDebug
#else
#define DEBUG if(0) qDebug
#endif

#ifdef DND_DEBUG
#define DNDDEBUG qDebug()
#else
#define DNDDEBUG if(0) qDebug()
#endif

static inline xcb_window_t xcb_window(QWindow *w)
{
    return static_cast<QXcbWindow *>(w->handle())->xcb_window();
}

xcb_atom_t toXdndAction(const QXcbDrag *drag, Qt::DropAction a)
{
    switch (a) {
    case Qt::CopyAction:
        return drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionCopy));
    case Qt::LinkAction:
        return drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionLink));
    case Qt::MoveAction:
    case Qt::TargetMoveAction:
        return drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionMove));
    case Qt::IgnoreAction:
        return XCB_NONE;
    default:
        return drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionCopy));
    }
}

void WindowEventHook::handleConfigureNotifyEvent(QXcbWindow *window, const xcb_configure_notify_event_t *event)
{
    DPlatformWindowHelper *helper = DPlatformWindowHelper::mapped.value(window);

    if (helper) {
        QWindowPrivate::get(window->window())->parentWindow = helper->m_frameWindow;
    }

    window->QXcbWindow::handleConfigureNotifyEvent(event);

    if (helper) {
        QWindowPrivate::get(window->window())->parentWindow = nullptr;

        if (helper->m_frameWindow->redirectContent())
            helper->m_frameWindow->markXPixmapToDirty(event->width, event->height);
    }
}

void WindowEventHook::handleMapNotifyEvent(QXcbWindow *window, const xcb_map_notify_event_t *event)
{
    window->QXcbWindow::handleMapNotifyEvent(event);

    if (DFrameWindow *frame = qobject_cast<DFrameWindow*>(window->window())) {
        frame->markXPixmapToDirty();
    } else if (DPlatformWindowHelper *helper = DPlatformWindowHelper::mapped.value(window)) {
        helper->m_frameWindow->markXPixmapToDirty();
    }
}

static Qt::DropAction toDropAction(QXcbConnection *c, xcb_atom_t a)
{
    if (a == c->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionCopy)) || a == 0)
        return Qt::CopyAction;
    if (a == c->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionLink)))
        return Qt::LinkAction;
    if (a == c->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionMove)))
        return Qt::MoveAction;

    return Qt::CopyAction;
}

void WindowEventHook::handleClientMessageEvent(QXcbWindow *window, const xcb_client_message_event_t *event)
{
    if (event->format != 32) {
        return window->QXcbWindow::handleClientMessageEvent(event);
    }

    do {
        if (event->type != window->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndPosition))
                && event->type != window->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndDrop))) {
            break;
        }

        QXcbDrag *drag = window->connection()->drag();

        // 不处理自己的drag事件
        if (drag->currentDrag()) {
            break;
        }

        int offset = 0;
        int remaining = 0;
        xcb_connection_t *xcb_connection = window->connection()->xcb_connection();
        Qt::DropActions support_actions = Qt::IgnoreAction;

        do {
            xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection, false, drag->xdnd_dragsource,
                                                                window->connection()->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndActionList)),
                                                                XCB_ATOM_ATOM, offset, 1024);
            xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection, cookie, NULL);
            if (!reply)
                break;

            remaining = 0;

            if (reply->type == XCB_ATOM_ATOM && reply->format == 32) {
                int len = xcb_get_property_value_length(reply)/sizeof(xcb_atom_t);
                xcb_atom_t *atoms = (xcb_atom_t *)xcb_get_property_value(reply);

                for (int i = 0; i < len; ++i) {
                    support_actions |= toDropAction(window->connection(), atoms[i]);
                }

                remaining = reply->bytes_after;
                offset += len;
            }

            free(reply);
        } while (remaining > 0);

        if (support_actions == Qt::IgnoreAction) {
            break;
        }

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        QMimeData *dropData = drag->platformDropData();
#else
        //FIXME(zccrs): must use the drop data object
        QMimeData *dropData = reinterpret_cast<QMimeData*>(drag->m_dropData);
#endif

        if (!dropData) {
            return;
        }

        dropData->setProperty("_d_dxcb_support_actions", QVariant::fromValue(support_actions));
    } while(0);

    if (event->type == window->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndDrop))) {
        QXcbDrag *drag = window->connection()->drag();

        DEBUG("xdndHandleDrop");
        if (!drag->currentWindow) {
            drag->xdnd_dragsource = 0;
            return; // sanity
        }

        const uint32_t *l = event->data.data32;

        DEBUG("xdnd drop");

        if (l[0] != drag->xdnd_dragsource) {
            DEBUG("xdnd drop from unexpected source (%x not %x", l[0], drag->xdnd_dragsource);
            return;
        }

        // update the "user time" from the timestamp in the event.
        if (l[2] != 0)
            drag->target_time = /*X11->userTime =*/ l[2];

        Qt::DropActions supported_drop_actions;
        QMimeData *dropData = 0;
        if (drag->currentDrag()) {
            dropData = drag->currentDrag()->mimeData();
            supported_drop_actions = Qt::DropActions(l[4]);
        } else {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            dropData = drag->platformDropData();
#else
            //FIXME(zccrs): must use the drop data object
            dropData = reinterpret_cast<QMimeData*>(drag->m_dropData);
#endif
            supported_drop_actions = drag->accepted_drop_action;

            // Drop coming from another app? Update keyboard modifiers.
            QGuiApplicationPrivate::modifier_buttons = QGuiApplication::queryKeyboardModifiers();
        }

        if (!dropData)
            return;
        // ###
        //        int at = findXdndDropTransactionByTime(target_time);
        //        if (at != -1)
        //            dropData = QDragManager::dragPrivate(X11->dndDropTransactions.at(at).object)->data;
        // if we can't find it, then use the data in the drag manager

        bool directSaveMode = dropData->hasFormat("XdndDirectSave0");

        dropData->setProperty("IsDirectSaveMode", directSaveMode);

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        // Drop coming from another app? Update buttons.
        if (!drag->currentDrag())
            QGuiApplicationPrivate::mouse_buttons = window->connection()->queryMouseButtons();

        QPlatformDropQtResponse response = QWindowSystemInterface::handleDrop(drag->currentWindow.data(),
                                                                            dropData, drag->currentPosition,
                                                                              supported_drop_actions,
                                                                              QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
#else
        QPlatformDropQtResponse response = QWindowSystemInterface::handleDrop(drag->currentWindow.data(),
                                                                            dropData, drag->currentPosition,
                                                                              supported_drop_actions);
#endif
        drag->setExecutedDropAction(response.acceptedAction());

        if (directSaveMode) {
            const QUrl &url = dropData->property("DirectSaveUrl").toUrl();

            if (url.isValid() && drag->xdnd_dragsource) {
                xcb_atom_t XdndDirectSaveAtom = Utility::internAtom("XdndDirectSave0");
                xcb_atom_t textAtom = Utility::internAtom("text/plain");
                QByteArray basename = Utility::windowProperty(drag->xdnd_dragsource, XdndDirectSaveAtom, textAtom, 1024);
                QByteArray fileUri = url.toString().toLocal8Bit() + "/" + basename;

                Utility::setWindowProperty(drag->xdnd_dragsource, XdndDirectSaveAtom,
                                           textAtom, fileUri.constData(), fileUri.length());

                Q_UNUSED(dropData->data("XdndDirectSave0"));
            }
        }

        xcb_client_message_event_t finished;
        finished.response_type = XCB_CLIENT_MESSAGE;
        finished.sequence = 0;
        finished.window = drag->xdnd_dragsource;
        finished.format = 32;
        finished.type = drag->atom(QXcbAtom::D_QXCBATOM_WRAPPER(XdndFinished));
        finished.data.data32[0] = drag->currentWindow ? xcb_window(drag->currentWindow.data()) : XCB_NONE;
        finished.data.data32[1] = response.isAccepted(); // flags
        finished.data.data32[2] = toXdndAction(drag, response.acceptedAction());
#ifdef Q_XCB_CALL
        Q_XCB_CALL(xcb_send_event(drag->xcb_connection(), false, drag->current_proxy_target,
                                  XCB_EVENT_MASK_NO_EVENT, (char *)&finished));
#else
        xcb_send_event(drag->xcb_connection(), false, drag->current_proxy_target,
                       XCB_EVENT_MASK_NO_EVENT, (char *)&finished);
#endif

        drag->xdnd_dragsource = 0;
        drag->currentWindow.clear();
        drag->waiting_for_status = false;

        // reset
        drag->target_time = XCB_CURRENT_TIME;
    } else {
        window->QXcbWindow::handleClientMessageEvent(event);
    }
}

void WindowEventHook::handleFocusInEvent(QXcbWindow *window, const xcb_focus_in_event_t *event)
{
    // Ignore focus events that are being sent only because the pointer is over
    // our window, even if the input focus is in a different window.
    if (event->detail == XCB_NOTIFY_DETAIL_POINTER)
        return;

    QWindow *w = static_cast<QWindowPrivate *>(QObjectPrivate::get(window->window()))->eventReceiver();

    if (DFrameWindow *frame = qobject_cast<DFrameWindow*>(w)) {
        if (frame->m_contentWindow)
            w = frame->m_contentWindow;
        else
            return;
    }

    VtableHook::callOriginalFun(window, &QXcbWindow::handleFocusInEvent, event);
}

void WindowEventHook::handleFocusOutEvent(QXcbWindow *window, const xcb_focus_out_event_t *event)
{
    // Ignore focus events
    if (event->mode == XCB_NOTIFY_MODE_GRAB) {
        return;
    }

    // Ignore focus events that are being sent only because the pointer is over
    // our window, even if the input focus is in a different window.
    if (event->detail == XCB_NOTIFY_DETAIL_POINTER)
        return;

    VtableHook::callOriginalFun(window, &QXcbWindow::handleFocusOutEvent, event);
}

void WindowEventHook::handlePropertyNotifyEvent(QXcbWindowEventListener *el, const xcb_property_notify_event_t *event)
{
    DQXcbWindow *window = reinterpret_cast<DQXcbWindow*>(static_cast<QXcbWindow*>(el));
    QWindow *ww = window->window();

    window->QXcbWindow::handlePropertyNotifyEvent(event);

    if (event->window == window->xcb_window()
            && event->atom == window->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_STATE))) {
        QXcbWindow::NetWmStates states = window->netWmStates();

        ww->setProperty(netWmStates, (int)states);

        if (const DFrameWindow *frame = qobject_cast<DFrameWindow*>(ww)) {
            if (frame->m_contentWindow)
                frame->m_contentWindow->setProperty(netWmStates, (int)states);
        }
    }
}

#ifdef XCB_USE_XINPUT22
static Qt::KeyboardModifiers translateModifiers(const QXcbKeyboard::_mod_masks &rmod_masks, int s)
{
    Qt::KeyboardModifiers ret = Qt::KeyboardModifiers();
    if (s & XCB_MOD_MASK_SHIFT)
        ret |= Qt::ShiftModifier;
    if (s & XCB_MOD_MASK_CONTROL)
        ret |= Qt::ControlModifier;
    if (s & rmod_masks.alt)
        ret |= Qt::AltModifier;
    if (s & rmod_masks.meta)
        ret |= Qt::MetaModifier;
    if (s & rmod_masks.altgr)
        ret |= Qt::GroupSwitchModifier;
    return ret;
}

static inline int fixed1616ToInt(FP1616 val)
{
    return int((qreal(val >> 16)) + (val & 0xFFFF) / (qreal)0xFFFF);
}

void WindowEventHook::handleXIEnterLeave(QXcbWindow *window, xcb_ge_event_t *event)
{
    DQXcbWindow *me = reinterpret_cast<DQXcbWindow *>(window);

    xXIEnterEvent *ev = reinterpret_cast<xXIEnterEvent *>(event);

    // Compare the window with current mouse grabber to prevent deliver events to any other windows.
    // If leave event occurs and the window is under mouse - allow to deliver the leave event.
    QXcbWindow *mouseGrabber = me->connection()->mouseGrabber();
    if (mouseGrabber && mouseGrabber != me
            && (ev->evtype != XI_Leave || QGuiApplicationPrivate::currentMouseWindow != me->window())) {
        return;
    }

    // Will send the press event repeatedly
    if (ev->evtype == XI_Enter && ev->mode == XCB_NOTIFY_MODE_UNGRAB) {
        if (ev->buttons_len > 0) {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            Qt::MouseButtons buttons = me->connection()->buttons();
#else
            Qt::MouseButtons buttons = me->connection()->buttonState();
#endif
            const Qt::KeyboardModifiers modifiers = translateModifiers(me->connection()->keyboard()->rmod_masks, ev->mods.effective_mods);
            unsigned char *buttonMask = (unsigned char *) &ev[1];
            for (int i = 1; i <= 15; ++i) {
                Qt::MouseButton b = me->connection()->translateMouseButton(i);

                if (b == Qt::NoButton)
                    continue;

                bool isSet = XIMaskIsSet(buttonMask, i);

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
                me->connection()->setButton(b, isSet);
#else
                me->connection()->setButtonState(b, isSet);
#endif

                const int event_x = fixed1616ToInt(ev->event_x);
                const int event_y = fixed1616ToInt(ev->event_y);
                const int root_x = fixed1616ToInt(ev->root_x);
                const int root_y = fixed1616ToInt(ev->root_y);

                if (buttons.testFlag(b)) {
                    if (!isSet) {
                        QGuiApplicationPrivate::lastCursorPosition = DHighDpi::fromNativePixels(QPointF(root_x, root_y), me->window());
                        me->handleButtonReleaseEvent(event_x, event_y, root_x, root_y,
                                                     0, modifiers, ev->time
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
                                                     , QEvent::MouseButtonRelease
#endif
                                                     );
                    }
                } /*else if (isSet) {
                    QGuiApplicationPrivate::lastCursorPosition = DHighDpi::fromNativePixels(QPointF(root_x, root_y), me->window());
                    me->handleButtonPressEvent(event_x, event_y, root_x, root_y,
                                               0, modifiers, ev->time
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
                                               , QEvent::MouseButtonPress
#endif
                                               );
                }*/
                //###(zccrs): 只设置button状态，不补发事件。防止窗口管理器（deepin-wm，kwin中无此问题）在鼠标Press期间触发此类型的事件，
                //            导致DTK窗口响应此点击事件后requestActive而将窗口显示到上层。此种情况发生在双击应用标题栏回复此窗口最大化状态时，
                //            如果此窗口下层为DTK窗口，则有可能触发此问题。
            }
        }
    }

    me->QXcbWindow::handleXIEnterLeave(event);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)

void WindowEventHook::windowEvent(QPlatformWindow *window, QEvent *event)
#else
bool WindowEventHook::windowEvent(QXcbWindow *window, QEvent *event)
#endif
{
    switch (event->type()) {
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::Drop: {
        DQDropEvent *ev = static_cast<DQDropEvent*>(event);
        Qt::DropActions support_actions = qvariant_cast<Qt::DropActions>(ev->mimeData()->property("_d_dxcb_support_actions"));

        if (support_actions != Qt::IgnoreAction) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            ev->m_actions = support_actions;
#else
            ev->act = support_actions;
#endif
        }
    }
    default:
        break;
    }

    return static_cast<QXcbWindow*>(window)->QXcbWindow::windowEvent(event);
}
#endif

DPP_END_NAMESPACE

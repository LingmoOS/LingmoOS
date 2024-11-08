/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "x11-window-manager.h"
#include "navigation-tab-bar.h"

#include <QWidget>
#include <QMouseEvent>

#include <QTabBar>
#include <QStyle>
#include <QStyleOptionTabBarBase>

#include <QApplication>
#include <QWindow>

#include <QDebug>

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>

#define START_DRAG_TIME 120

static X11WindowManager *global_instance = nullptr;

X11WindowManager *X11WindowManager::getInstance()
{
    if (!global_instance) {
        global_instance = new X11WindowManager;
    }
    return global_instance;
}

X11WindowManager::X11WindowManager(QObject *parent) : QObject(parent)
{

}

bool X11WindowManager::eventFilter(QObject *watched, QEvent *event)
{
    //qDebug()<<event->type();

    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchCancel:
    case QEvent::TouchEnd: {
        qDebug()<<event->type();
        break;
    }

    case QEvent::MouseButtonPress: {

        //Make tabbar blank area dragable and do not effect tablabel.
        if (qobject_cast<QTabBar *>(watched)) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            QTabBar *tabBar = qobject_cast<QTabBar *>(watched);
            if (tabBar->tabAt(e->pos()) != -1)
                return false;
        }
        QMouseEvent *e = static_cast<QMouseEvent *>(event);

        if (QObject::eventFilter(watched, event))
            return true;
        if (e->button() == Qt::LeftButton) {
            m_prepare_drag_time = e->timestamp();
            //修改页签可以拖拽，记录鼠标位置
            //bug#94981 修改页签可以拖拽，记录鼠标位置
            m_press_pos = QCursor::pos();
            m_is_draging = true;
            m_current_widget = static_cast<QWidget *>(watched);
            m_toplevel_offset = m_current_widget->topLevelWidget()->mapFromGlobal(m_press_pos);
        }

        //qDebug()<<event->type();

        break;
    }
    case QEvent::MouseMove: {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);

        //qDebug()<<e->type()<<e->pos();

        if (m_is_draging) {
            if (QX11Info::isPlatformX11()) {
                bool isTouchMove = e->source() == Qt::MouseEventSynthesizedByQt;

                QPoint currentPos = QCursor::pos();
                QPoint offset = QCursor::pos() - m_press_pos;
                bool smallOffset = qAbs(offset.x()) <= 2 && qAbs(offset.y() <= 2);
                if (smallOffset)
                    break;

                Display *display = QX11Info::display();
                Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
                XEvent xEvent;
                const auto pos = currentPos - offset;

                memset(&xEvent, 0, sizeof(XEvent));
                xEvent.xclient.type = ClientMessage;
                xEvent.xclient.message_type = netMoveResize;
                xEvent.xclient.display = display;
                xEvent.xclient.window = m_current_widget->topLevelWidget()->winId();
                xEvent.xclient.format = 32;
                xEvent.xclient.data.l[0] = pos.x() * qApp->devicePixelRatio();
                xEvent.xclient.data.l[1] = pos.y() * qApp->devicePixelRatio();
                xEvent.xclient.data.l[2] = 8;
                xEvent.xclient.data.l[3] = Button1;
                xEvent.xclient.data.l[4] = 0;

                XUngrabPointer(display, CurrentTime);
                XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()),
                           False, SubstructureNotifyMask | SubstructureRedirectMask,
                           &xEvent);
                XFlush(display);

                m_is_draging = false;

                //NOTE: use x11 move will ungrab the window focus
                //hide and show will restore the focus and it seems
                //there is no bad effect for explorer main window.
                if (isTouchMove) {
                    if (!m_current_widget->mouseGrabber()) {
                        m_current_widget->grabMouse();
                        m_current_widget->releaseMouse();
                    }
                }

                if (qobject_cast<QTabBar *>(m_current_widget)) {
                    m_current_widget->hide();
                    m_current_widget->show();
                }

                //balance mouse release event
                QMouseEvent me(QMouseEvent::MouseButtonRelease, e->pos(), e->windowPos(), e->screenPos(), e->button(), e->buttons(), e->modifiers(), Qt::MouseEventSynthesizedByApplication);
                qApp->sendEvent(watched, &me);

                memset(&xEvent, 0, sizeof(XEvent));
                xEvent.type = ButtonRelease;
                xEvent.xbutton.button = Button1;
                xEvent.xbutton.same_screen = true;
                xEvent.xbutton.window = m_current_widget->topLevelWidget()->winId();
                XSendEvent(display, m_current_widget->topLevelWidget()->winId(),
                           False, 0,
                           &xEvent);

                return true;
            } else {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                auto widget = qobject_cast<QWidget *>(watched);
                auto topLevel = widget->topLevelWidget();
                topLevel->windowHandle()->startSystemMove();
                m_is_draging = false;
#else
                //auto me = static_cast<QMouseEvent *>(event);
                auto widget = qobject_cast<QWidget *>(watched);
                auto topLevel = widget->topLevelWidget();
                auto globalPos = QCursor::pos();
                //auto offset = globalPos - m_press_pos;
                topLevel->move(globalPos - m_toplevel_offset);
#endif
            }
        }
        //commit id: 339dbaf18b9555d274e69c0589a755457e3f555b [FIX] 【文件管理器】首页页签未按UI设计稿还原 [LINK]94981
        /*else {
            bool overDragTime = 0 < m_prepare_drag_time && (m_prepare_drag_time + START_DRAG_TIME) < e->timestamp();
            bool canDrag = !isTouchMove || overDragTime;
            if (canDrag) {
                m_is_draging = true;
                m_prepare_drag_time = 0;
                m_press_pos = QCursor::pos();
                m_current_widget = static_cast<QWidget *>(watched);
                m_toplevel_offset = m_current_widget->topLevelWidget()->mapFromGlobal(m_press_pos);
            }
        }*/
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto me = static_cast<QMouseEvent *>(event);

        if (me->source() == Qt::MouseEventSynthesizedByApplication)
            break;

        m_prepare_drag_time = 0;
        m_press_pos = QPoint();
        m_is_draging = false;
        m_current_widget = nullptr;
        break;
    }
    default:
        return false;
    }
    return false;
}

void X11WindowManager::registerWidget(QWidget *widget)
{
    widget->removeEventFilter(this);
    widget->installEventFilter(this);
}

static XAtomHelper *global_instance1 = nullptr;

XAtomHelper *XAtomHelper::getInstance()
{
    if (!global_instance1)
        global_instance1 = new XAtomHelper;
    return global_instance1;
}

bool XAtomHelper::isFrameLessWindow(int winId)
{
    auto hints = getInstance()->getWindowMotifHint(winId);
    if (hints.flags == MWM_HINTS_DECORATIONS && hints.functions == 1) {
        return true;
    }
    return false;
}

bool XAtomHelper::isWindowDecorateBorderOnly(int winId)
{
    return isWindowMotifHintDecorateBorderOnly(getInstance()->getWindowMotifHint(winId));
}

bool XAtomHelper::isWindowMotifHintDecorateBorderOnly(const MotifWmHints &hint)
{
    bool isDeco = false;
    if (hint.flags & MWM_HINTS_DECORATIONS && hint.flags != MWM_HINTS_DECORATIONS) {
        if (hint.decorations == MWM_DECOR_BORDER)
            isDeco = true;
    }
    return isDeco;
}

bool XAtomHelper::isLINGMOCsdSupported()
{
    // fixme:
    return false;
}

bool XAtomHelper::isLINGMODecorationWindow(int winId)
{
    if (m_lingmoDecorationAtion == None)
        return false;

    Atom type;
    int format;
    ulong nitems;
    ulong bytes_after;
    uchar *data;

    bool isLINGMODecoration = false;

    XGetWindowProperty(QX11Info::display(), winId, m_lingmoDecorationAtion,
                       0, LONG_MAX, false,
                       m_lingmoDecorationAtion, &type,
                       &format, &nitems,
                       &bytes_after, &data);

    if (type == m_lingmoDecorationAtion) {
        if (nitems == 1) {
            isLINGMODecoration = data[0];
        }
    }

    return isLINGMODecoration;
}

UnityCorners XAtomHelper::getWindowBorderRadius(int winId)
{
    UnityCorners corners;

    Atom type;
    int format;
    ulong nitems;
    ulong bytes_after;
    uchar *data;

    if (m_unityBorderRadiusAtom != None) {
        XGetWindowProperty(QX11Info::display(), winId, m_unityBorderRadiusAtom,
                           0, LONG_MAX, false,
                           XA_CARDINAL, &type,
                           &format, &nitems,
                           &bytes_after, &data);

        if (type == XA_CARDINAL) {
            if (nitems == 4) {
                corners.topLeft = static_cast<ulong>(data[0]);
                corners.topRight = static_cast<ulong>(data[1*sizeof (ulong)]);
                corners.bottomLeft = static_cast<ulong>(data[2*sizeof (ulong)]);
                corners.bottomRight = static_cast<ulong>(data[3*sizeof (ulong)]);
            }
            XFree(data);
        }
    }

    return corners;
}

void XAtomHelper::setWindowBorderRadius(int winId, const UnityCorners &data)
{
    if (m_unityBorderRadiusAtom == None)
        return;

    ulong corners[4] = {data.topLeft, data.topRight, data.bottomLeft, data.bottomRight};

    XChangeProperty(QX11Info::display(), winId, m_unityBorderRadiusAtom, XA_CARDINAL,
                    32, XCB_PROP_MODE_REPLACE, (const unsigned char *) &corners, sizeof (corners)/sizeof (corners[0]));
}

void XAtomHelper::setWindowBorderRadius(int winId, int topLeft, int topRight, int bottomLeft, int bottomRight)
{
    if (m_unityBorderRadiusAtom == None)
        return;

    ulong corners[4] = {(ulong)topLeft, (ulong)topRight, (ulong)bottomLeft, (ulong)bottomRight};

    XChangeProperty(QX11Info::display(), winId, m_unityBorderRadiusAtom, XA_CARDINAL,
                    32, XCB_PROP_MODE_REPLACE, (const unsigned char *) &corners, sizeof (corners)/sizeof (corners[0]));
}

void XAtomHelper::setLINGMODecoraiontHint(int winId, bool set)
{
    if (m_lingmoDecorationAtion == None)
        return;

    XChangeProperty(QX11Info::display(), winId, m_lingmoDecorationAtion, m_lingmoDecorationAtion, 32, XCB_PROP_MODE_REPLACE, (const unsigned char *) &set, 1);
}

void XAtomHelper::setWindowMotifHint(int winId, const MotifWmHints &hints)
{
    if (m_unityBorderRadiusAtom == None)
        return;

    XChangeProperty(QX11Info::display(), winId, m_motifWMHintsAtom, m_motifWMHintsAtom,
                    32, XCB_PROP_MODE_REPLACE, (const unsigned char *)&hints, sizeof (MotifWmHints)/ sizeof (ulong));
}

MotifWmHints XAtomHelper::getWindowMotifHint(int winId)
{
    MotifWmHints hints;

    if (m_unityBorderRadiusAtom == None)
        return hints;

    uchar *data;
    Atom type;
    int format;
    ulong nitems;
    ulong bytes_after;

    XGetWindowProperty(QX11Info::display(), winId, m_motifWMHintsAtom,
                       0, sizeof (MotifWmHints)/sizeof (long), false, AnyPropertyType, &type,
                       &format, &nitems, &bytes_after, &data);

    if (type == None) {
        return hints;
    } else {
        hints = *(MotifWmHints *)data;
        XFree(data);
    }
    return hints;
}

XAtomHelper::XAtomHelper(QObject *parent) : QObject(parent)
{
    if (!QX11Info::isPlatformX11())
        return;

    m_motifWMHintsAtom = XInternAtom(QX11Info::display(), "_MOTIF_WM_HINTS", true);
    m_unityBorderRadiusAtom = XInternAtom(QX11Info::display(), "_UNITY_GTK_BORDER_RADIUS", false);
    m_lingmoDecorationAtion = XInternAtom(QX11Info::display(), "_KWIN_LINGMO_DECORAION", false);
}

Atom XAtomHelper::registerLINGMOCsdNetWmSupportAtom()
{
    // fixme:
    return None;
}

void XAtomHelper::unregisterLINGMOCsdNetWmSupportAtom()
{
    // fixme:
}

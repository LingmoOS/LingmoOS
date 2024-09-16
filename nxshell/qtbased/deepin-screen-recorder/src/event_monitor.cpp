// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "event_monitor.h"
#include "keydefine.h"
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

EventMonitor::EventMonitor(QObject *parent) : QThread(parent)
{
    isPress = false;
    if (Utils::isWaylandMode) {
        initWaylandEventMonitor();
    }
}

EventMonitor::~EventMonitor()
{

}

void EventMonitor::releaseRes()
{
    if (m_display_datalink && m_display) {
        qInfo() << __FUNCTION__ << __LINE__ << "执行 XRecordDisableContext ...";
        XRecordDisableContext(m_display, m_context);
        qInfo() << __FUNCTION__ << __LINE__ << "执行 XRecordFreeContext ...";
        XRecordFreeContext(m_display, m_context);
        qInfo() << __FUNCTION__ << __LINE__ << "执行 XSync ...";
        XSync(m_display, False);
        qInfo() << __FUNCTION__ << __LINE__ << "执行 XCloseDisplay m_display_datalink...";
        XCloseDisplay(m_display_datalink);
        qInfo() << __FUNCTION__ << __LINE__ << "执行 XCloseDisplay m_display...";
        XCloseDisplay(m_display);
        m_display_datalink = nullptr;
        m_display = nullptr;
    }
}

void EventMonitor::run()
{
    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        fprintf(stderr, "unable to open display\n");
        return;
    }

    // Receive from ALL clients, including future clients.
    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *range = XRecordAllocRange();
    if (range == nullptr) {
        fprintf(stderr, "unable to allocate XRecordRange\n");
        return;
    }

    // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events.
    memset(range, 0, sizeof(XRecordRange));
    range->device_events.first = KeyPress;
    range->device_events.last  = MotionNotify;

    // And create the XRECORD context.
    m_context = XRecordCreateContext(m_display, 0, &clients, 1, &range, 1);
    if (m_context == 0) {
        fprintf(stderr, "XRecordCreateContext failed\n");
        return;
    }
    XFree(range);

    XSync(m_display, True);

    m_display_datalink = XOpenDisplay(nullptr);
    if (m_display_datalink == nullptr) {
        fprintf(stderr, "unable to open second display\n");
        return;
    }

    if (!XRecordEnableContext(m_display_datalink, m_context,  callback, reinterpret_cast<XPointer>(this))) {
        fprintf(stderr, "XRecordEnableContext() failed\n");
        return;
    }
}

void EventMonitor::callback(XPointer ptr, XRecordInterceptData *data)
{
    (reinterpret_cast<EventMonitor *>(ptr))->handleEvent(data);
}

void EventMonitor::handleEvent(XRecordInterceptData *data)
{
    if (data->category == XRecordFromServer) {

        xEvent *event = reinterpret_cast<xEvent *>(data->data);
        switch (event->u.u.type) {
        case ButtonPress:
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                isPress = true;
                //鼠标按压
                emit mousePress(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            } else if (event->u.u.detail == WheelUp || event->u.u.detail == WheelDown) {
                int time = int (QDateTime::currentDateTime().toTime_t());
                //鼠标滚动
                //emit mouseScroll(static_cast<int>(event->u.enterLeave.time), event->u.u.detail, event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
                emit mouseScroll(time, event->u.u.detail, event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case MotionNotify:
            if (isPress) {
                emit mouseDrag(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            } else {
                //鼠标移动
                emit mouseMove(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case ButtonRelease:
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                isPress = false;
                emit activateWindow();
                emit mouseRelease(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case KeyPress:
            // If key is equal to esc, emit pressEsc signal.
//            if ((reinterpret_cast<unsigned char *>(data->data))[1] == KEY_ESCAPE) {
//                emit pressEsc();
//            } else {
//                emit pressKeyButton((reinterpret_cast<unsigned char *>(data->data))[1]);
//            }

            //键盘按键按下
            emit keyboardPress((reinterpret_cast<unsigned char *>(data->data))[1]);
            break;
        case KeyRelease:
            //键盘按键释放
            emit keyboardRelease((reinterpret_cast<unsigned char *>(data->data))[1]);
            break;
        default:
            break;
        }
    }

    fflush(stdout);
    XRecordFreeData(data);
}

XFixesCursorImage *EventMonitor::getCursorImage()
{
    /*
        if(Utils::isWaylandMode) {
            return nullptr;
        }
    */
    Display *x11Display = XOpenDisplay(nullptr);
    if (!x11Display) {
        fprintf(stderr, "unable to open display\n");
        return nullptr;
    }
    return XFixesGetCursorImage(x11Display);
}

//wayland截屏下获取光标的图片
QImage EventMonitor::getCursorImageWayland()
{
    QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                 QStringLiteral("/KWin"),
                                 QStringLiteral("org.kde.KWin"));

    QPixmap res;
    QDBusReply<QByteArray> reply = kwinInterface.call(QStringLiteral("cursorImage"));
    qDebug() << "reply: " << reply.isValid();
    qDebug() << "reply.value().isNull(): " << reply.value().isNull();
    qDebug() << "reply.value().isNull(): " << reply.value().isEmpty();
    res.loadFromData(reply.value());
    if (!res.isNull()) {
        QFile dbusResult(reply.value());
        dbusResult.remove();
    }
    return res.toImage();

}

void EventMonitor::initWaylandEventMonitor()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.interface()->isServiceRegistered("com.deepin.daemon.InputDevices")) {
        qDebug() << "DBusError" << "com.deepin.daemon.InputDevices";
        return;
    }

    // 监控全局鼠标信号。
    sessionBus.connect("com.deepin.daemon.InputDevices",
                       "/com/deepin/api/XEventMonitor", "com.deepin.api.XEventMonitor", "ButtonPress",
                       this, SLOT(ButtonPressEvent(int, int, int, QString)));

    sessionBus.connect("com.deepin.daemon.InputDevices",
                       "/com/deepin/api/XEventMonitor", "com.deepin.api.XEventMonitor", "ButtonRelease",
                       this, SLOT(ButtonReleaseEvent(int, int, int, QString)));

    sessionBus.connect("com.deepin.daemon.InputDevices",
                       "/com/deepin/api/XEventMonitor", "com.deepin.api.XEventMonitor", "CursorMove",
                       this, SLOT(CursorMoveEvent(int, int, QString)));
}

void EventMonitor::ButtonPressEvent(int type, int x, int y, QString str)
{
    Q_UNUSED(str);
    // Button1为鼠标左键，Button3为鼠标右键
    if (type == Button1 || type == Button3)
        emit mousePress(x, y);
    //Debug()<<"=====ButtonPressEvent=====";
}

void EventMonitor::ButtonReleaseEvent(int type, int x, int y, QString str)
{
    Q_UNUSED(str);
    // Button1为鼠标左键，Button3为鼠标右键
    if (type == Button1 || type == Button3)
        emit mouseRelease(x, y);
    //qDebug()<<"=====ButtonReleaseEvent=====";
}
//  Wayland下全局鼠标移动事件通过Dbus信号接收
void EventMonitor::CursorMoveEvent(int x, int y, QString str)
{
    Q_UNUSED(str);
    emit mouseMove(x, y);
}

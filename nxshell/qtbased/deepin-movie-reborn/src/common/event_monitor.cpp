// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#undef Bool
#include <QCursor>

#include "event_monitor.h"
#define Bool int
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/extensions/record.h>
#undef Bool

//cppcheck 修改
void callback(XPointer ptr, XRecordInterceptData *pData)
{
    (reinterpret_cast<dmr::EventMonitor *>(ptr))->handleRecordEvent(pData);
}

namespace dmr {

EventMonitor::EventMonitor(QObject *parent) : QThread(parent)
{
    m_bIsPress = false;
}

//cppcheck 误报
void EventMonitor::run()
{
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        fprintf(stderr, "unable to open display\n");
        return;
    }
    // Receive from ALL clients, including future clients.
    //XRecordClientSpec clients = XRecordAllClients;
    XRecordClientSpec clients = XRecordCurrentClients;
    XRecordRange *pRange = XRecordAllocRange();
    if (pRange == nullptr) {
        fprintf(stderr, "unable to allocate XRecordRange\n");
        return;
    }

    memset(pRange, 0, sizeof(XRecordRange));
    pRange->device_events.first = ButtonPress;
    pRange->device_events.last  = MotionNotify;

    // And create the XRECORD context.
    XRecordContext context = XRecordCreateContext(display, 0, &clients, 1, &pRange, 1);
    if (context == 0) {
        fprintf(stderr, "XRecordCreateContext failed\n");
        return;
    }
    XFree(pRange);

    XSync(display, True);

    Display *display_datalink = XOpenDisplay(nullptr);
    if (display_datalink == nullptr) {
        fprintf(stderr, "unable to open second display\n");
        return;
    }

    if (!XRecordEnableContext(display_datalink, context,  callback, (XPointer) this)) {
        fprintf(stderr, "XRecordEnableContext() failed\n");
        return;
    }
}

void EventMonitor::handleRecordEvent(void *pValue)
{
    XRecordInterceptData *pData = (XRecordInterceptData *)pValue;
    if (!m_recording) {
        XRecordFreeData(pData);
        return;
    }

    if (pData->category == XRecordFromServer) {
        xEvent *event = (xEvent *)pData->data;
        switch (event->u.u.type) {
        case ButtonPress:
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                m_bIsPress = true;
                emit buttonedPress(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case MotionNotify:
            if (m_bIsPress) {
                emit buttonedDrag(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        case ButtonRelease:
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                m_bIsPress = false;
                emit buttonedRelease(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            }
            break;
        default:
            break;
        }
    }

    XRecordFreeData(pData);
}

void EventMonitor::resumeRecording()
{
    if (!m_recording) {
        m_recording = 1;
    }
}

void EventMonitor::suspendRecording()
{
    if (m_recording) {
        if (m_bIsPress) {
            m_bIsPress = false;
            QPoint pos = QCursor::pos();
            emit buttonedRelease(pos.x(), pos.y());
        }
        m_recording = 0;
    }
}

}


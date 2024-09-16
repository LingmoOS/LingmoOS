// Copyright (C) 2022 Uniontech Technology Co., Ltd.
// Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "recordeventmonitor.h"
#include "workspace.h"

#include <dlfcn.h>
#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>

// The PROXIMITYIN and PROXIMITYOUT enumeration value is the same as the event enumeration value in the EventToCore function in xorg.
#define PROXIMITYIN     15
#define PROXIMITYOUT    16
#define TOUCHDOWN      (LASTEvent + 1)
#define TOUCHMOTION    (LASTEvent + 2)
#define TOUCHUP        (LASTEvent + 3)

static void callback(XPointer trash, XRecordInterceptData *data);
void handleRecordEvent(XRecordInterceptData *);
int errorHandler(Display *dpy, XErrorEvent *ev);
int IOErrorHandler(Display *dpy);
void IOErrorExitHandler(Display *dpy, void *data);
XRecordContext m_context;
Display *m_d0 = nullptr;
bool m_bFlag = false;
int m_eventTime = 0;

Q_LOGGING_CATEGORY(KWIN_RECORDEVENTMONITOR, "kwin_record_event_monitor", QtWarningMsg)

RecordEventMonitor::RecordEventMonitor(QObject *parent)
    : QThread(parent)
{
    connect(KWin::Workspace::self(), &KWin::Workspace::touchDownByRecord, this, &RecordEventMonitor::touchDown);
    connect(KWin::Workspace::self(), &KWin::Workspace::touchMotionByRecord, this, &RecordEventMonitor::touchMotion);
    connect(KWin::Workspace::self(), &KWin::Workspace::touchUpByRecord, this, &RecordEventMonitor::touchUp);
    connect(KWin::Workspace::self(), &KWin::Workspace::buttonReleaseByRecord, this, &RecordEventMonitor::buttonRelease);
    connect(KWin::Workspace::self(), &KWin::Workspace::motionByRecord, this, &RecordEventMonitor::motion);
}

void RecordEventMonitor::run()
{
    Display *pDisplay = XOpenDisplay(nullptr);
    m_d0 = pDisplay;
    if (pDisplay == nullptr)
        return;

    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *pRange = XRecordAllocRange();
    if (pRange == nullptr)
        return;

    // In this, we just get touch event in a range.
    memset(pRange, 0, sizeof(XRecordRange));
    pRange->device_events.first = KeyPress;
    pRange->device_events.last  = TOUCHUP;

    XRecordContext context = XRecordCreateContext(pDisplay, 0, &clients, 1, &pRange, 1);
    m_context = context;
    if (context == 0)
        return;

    XFree(pRange);
    XSync(pDisplay, True);

    Display* displayDatalink = XOpenDisplay(nullptr);
    if (displayDatalink == nullptr)
        return;

    void *file = dlopen("libX11.so", RTLD_LAZY);
    if (file) {
        void *function = dlsym(file, "XSetErrorHandler");
        if (function) {
            void (*handler)(decltype(errorHandler)) = reinterpret_cast<decltype(handler)>(function);
            handler(errorHandler);
        } else {
            qWarning(KWIN_RECORDEVENTMONITOR) << "Fail to find function 'XSetErrorHandler'";
        }

        function = dlsym(file, "XSetIOErrorHandler");
        if (function) {
            void (*handler)(decltype(IOErrorHandler)) = reinterpret_cast<decltype(handler)>(function);
            handler(IOErrorHandler);
        } else {
            qWarning(KWIN_RECORDEVENTMONITOR) << "Fail to find function 'XSetIOErrorHandler'";
        }

        function = dlsym(file, "XSetIOErrorExitHandler");
        if (function) {
            void (*handler)(Display *, decltype(IOErrorExitHandler), void *) = reinterpret_cast<decltype(handler)>(function);
            handler(displayDatalink, IOErrorExitHandler, nullptr);
        } else {
            qWarning(KWIN_RECORDEVENTMONITOR) << "Fail to find function 'XSetIOErrorExitHandler'";
        }

        dlclose(file);
    } else {
        qWarning(KWIN_RECORDEVENTMONITOR) << "Errors in function dlopen:" << dlerror();
    }

    if (!XRecordEnableContext(displayDatalink, context, callback, (XPointer) this))
        return;
}

void callback(XPointer ptr, XRecordInterceptData *data)
{
    handleRecordEvent(data);
}

void handleRecordEvent(XRecordInterceptData* data)
{
    if (data->category == XRecordFromServer) {
        xEvent * event = reinterpret_cast<xEvent *>(data->data);
        switch (event->u.u.type) {
        case ButtonRelease:
            if (m_bFlag) {
                Q_EMIT KWin::Workspace::self()->buttonReleaseByRecord();
            }
            break;
        case MotionNotify:
            if (m_bFlag) {
                Q_EMIT KWin::Workspace::self()->motionByRecord();
            }
            break;
        case PROXIMITYIN:
            // When using a TabletTool device, this event is raised first
            m_bFlag = true;
            break;
        case PROXIMITYOUT:
            // This event is raised when the TabletTool device is left.
            m_bFlag = false;
            break;
        case TOUCHDOWN:
            // sometimes, xrecord extend will get repeated touch down event(maybe send to the real client).
            // but, those touch down event is belong to same ancestors.
            // so, if you need, you can use (event->u.keyButtonPointer.time)
            // to filter out the repeated touh down event.
            if (m_eventTime != event->u.keyButtonPointer.time) {
                Q_EMIT KWin::Workspace::self()->touchDownByRecord(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY, event->u.keyButtonPointer.time);
                m_eventTime = event->u.keyButtonPointer.time;
            }
            break;
        case TOUCHMOTION:
            Q_EMIT KWin::Workspace::self()->touchMotionByRecord(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY, event->u.keyButtonPointer.time);
            break;
        case TOUCHUP:
            Q_EMIT KWin::Workspace::self()->touchUpByRecord(event->u.keyButtonPointer.time);
            break;
        default:
            break;
        }
    }

    fflush(stdout);
    XRecordFreeData(data);
}

void RecordEventMonitor::stopRecord()
{
    XRecordDisableContext(m_d0, m_context);
    XRecordFreeContext(m_d0, m_context);
    XSync(m_d0, True);
    XCloseDisplay(m_d0);
}

int errorHandler(Display *dpy, XErrorEvent *ev)
{
    qCWarning(KWIN_RECORDEVENTMONITOR) << "Error occurred in RecordEventMonitor";
    return 1;
}

int IOErrorHandler(Display *dpy)
{
    qCWarning(KWIN_RECORDEVENTMONITOR) << "Fatal IO error in RecordEventMonitor";
    return 1;
}

void IOErrorExitHandler(Display *dpy, void *data)
{
    qCWarning(KWIN_RECORDEVENTMONITOR) << "Exit RecordEventMonitor due to fatal IO error";
}

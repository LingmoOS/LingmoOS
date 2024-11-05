/**
 * Copyright (C) 2018 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "cursormonitor.h"
#include <X11/Xlib.h>
#include <QPoint>

CursorMonitor::CursorMonitor(QObject *parent) : QThread(parent)
{

}

void CursorMonitor::run()
{
    Display *display;
    XEvent xevent;
    Window window;

    display = XOpenDisplay(NULL);


    window = DefaultRootWindow(display);
    XAllowEvents(display, AsyncBoth, CurrentTime);

    XGrabPointer(display,
                 window,
                 1,
                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask ,
                 GrabModeAsync,
                 GrabModeAsync,
                 None,
                 None,
                 CurrentTime);

    while(1) {
        XNextEvent(display, &xevent);

        switch (xevent.type) {
            case MotionNotify:
            //printf("Mouse move      : [%d, %d]\n", xevent.xmotion.x_root, xevent.xmotion.y_root);
                Q_EMIT cursorPosChanged(QPoint(xevent.xmotion.x_root, xevent.xmotion.y_root));
            break;
            case ButtonPress:
//              printf("Button pressed  : %s\n", key_name[xevent.xbutton.button - 1]);
            break;
            case ButtonRelease:
//              printf("Button released : %s\n", key_name[xevent.xbutton.button - 1]);
            break;
        }
    }
}

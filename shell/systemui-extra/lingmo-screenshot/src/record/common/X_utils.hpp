#pragma once
#include <QX11Info>
#include <QPoint>
#include "my_x.h"

// replacement for QX11Info::isPlatformX11()
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
inline bool IsPlatformX11() {
    char *v = getenv("XDG_SESSION_TYPE");
    return (v == NULL || strcasecmp(v, "x11") == 0);
}
#else
inline bool IsPlatformX11() {
    char *v = getenv("XDG_SESSION_TYPE");
    //return true;
    return ((v == NULL || strcasecmp(v, "x11") == 0) && QX11Info::isPlatformX11());
}
#endif

QPoint GetMousePhysicalCoordinates()
{
    if(IsPlatformX11()) {
        Window root, child;
        int root_x, root_y;
        int win_x, win_y;
        unsigned int mask_return;
        XQueryPointer(QX11Info::display(), QX11Info::appRootWindow(), &root, &child, &root_x, &root_y, &win_x, &win_y, &mask_return);
        return QPoint(root_x, root_y);
    } else {
        return QPoint(0, 0); // TODO: implement for wayland
    }
}

// Tries to find the real window that corresponds to a top-level window (the actual window without window manager decorations).
// Returns None if it can't find the window (probably because the window is not handled by the window manager).
// Based on the xprop source code (http://cgit.freedesktop.org/xorg/app/xprop/tree/clientwin.c).
Window X11FindRealWindow(Display* display, Window window) {

    // is this the real window?
    Atom actual_type;
    int actual_format;
    unsigned long items, bytes_left;
    unsigned char *data = NULL;
    XGetWindowProperty(display, window, XInternAtom(display, "WM_STATE", true),
                       0, 0, false, AnyPropertyType, &actual_type, &actual_format, &items, &bytes_left, &data);
    if(data != NULL)
        XFree(data);
    if(actual_type != None)
        return window;

    // get the child windows
    Window root, parent, *childs;
    unsigned int childcount;
    if(!XQueryTree(display, window, &root, &parent, &childs, &childcount)) {
        return None;
    }

    // recursively call this function for all childs
    Window real_window = None;
    for(unsigned int i = childcount; i > 0; ) {
        --i;
        Window w = X11FindRealWindow(display, childs[i]);
        if(w != None) {
            real_window = w;
            break;
        }
    }

    // free child window list
    if(childs != NULL)
        XFree(childs);

    return real_window;

}

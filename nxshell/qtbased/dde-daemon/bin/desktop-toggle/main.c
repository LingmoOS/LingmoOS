// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    Display *disp;
    Window root;
    Atom showing_desktop_atom, actual_type;
    int actual_format, error, current = 0;
    unsigned long nitems, after;
    unsigned char *data = NULL;

    /* Open the default display */
    if(!(disp = XOpenDisplay(NULL))) {
        fprintf(stderr, "Cannot open display.\n");
        return -1;
    }

    /* This is the default root window */
    root = DefaultRootWindow(disp);

    /* find the Atom for _NET_SHOWING_DESKTOP */
    showing_desktop_atom = XInternAtom(disp, "_NET_SHOWING_DESKTOP", False);

    /* Obtain the current state of _NET_SHOWING_DESKTOP on the default root window */
    error = XGetWindowProperty(disp, root, showing_desktop_atom, 0, 1, False, XA_CARDINAL,
                               &actual_type, &actual_format, &nitems, &after, &data);
    if(error != Success) {
        fprintf(stderr, "Get '_NET_SHOWING_DESKTOP' property error %d!\n", error);
        XCloseDisplay(disp);
        return -1;
    }

    /* The current state should be in data[0] */
    if(data) {
        current = data[0];
        XFree(data);
        data = NULL;
    }
    printf("Current state: %d\n", current);

    /* If nitems is 0, forget about data[0] and assume that current should be False */
    if(!nitems) {
        fprintf(stderr, "Unexpected result.\n");
        fprintf(stderr, "Assuming unshown desktop!\n");
        current = False;
    }

    /* Initialize Xevent struct */
    XEvent xev = {
        .xclient = {
            .type = ClientMessage,
            .send_event = True,
            .display = disp,
            .window = root,
            .message_type = showing_desktop_atom,
            .format = 32,
            .data.l[0] = !current /* That’s what we want the new state to be */
        }
    };

    /* Send the event to the window manager */
    XSendEvent(disp, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

    /* Output the new state ("visible" or "hidden") so the calling program
     * can react accordingly.
     */
    /* printf("%s\n", current ? "hidden" : "visible"); */

    XCloseDisplay(disp);
    return 0;
}

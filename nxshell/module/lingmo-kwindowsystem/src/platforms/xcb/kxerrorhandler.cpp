/*

  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

*/

#include "kxerrorhandler_p.h"
#include <config-kwindowsystem.h>

#include "netwm_def.h"

#include <stdio.h>

#include <QByteArray>
#include <QString>

class KXErrorHandlerPrivate
{
public:
    KXErrorHandlerPrivate(Display *dpy) :
        first_request(XNextRequest(dpy)),
        display(dpy),
        was_error(false)
    {
    }
    unsigned long first_request;
    Display *display;
    bool was_error;
    XErrorEvent error_event;
};

KXErrorHandler **KXErrorHandler::handlers = nullptr;
int KXErrorHandler::pos = 0;
int KXErrorHandler::size = 0;

KXErrorHandler::KXErrorHandler(Display *dpy)
    :   user_handler1(nullptr),
        user_handler2(nullptr),
        old_handler(XSetErrorHandler(handler_wrapper)),
        d(new KXErrorHandlerPrivate(dpy))
{
    addHandler();
}

KXErrorHandler::KXErrorHandler(int (*handler)(Display *, XErrorEvent *), Display *dpy)
    :   user_handler1(nullptr),
        user_handler2(handler),
        old_handler(XSetErrorHandler(handler_wrapper)),
        d(new KXErrorHandlerPrivate(dpy))
{
    addHandler();
}

KXErrorHandler::~KXErrorHandler()
{
    XSetErrorHandler(old_handler);
    Q_ASSERT_X(this == handlers[ pos - 1 ], "KXErrorHandler", "out of order");
    --pos;
    delete d;
}

void KXErrorHandler::addHandler()
{
    if (size == pos) {
        size += 16;
        handlers = static_cast< KXErrorHandler ** >(realloc(handlers, size * sizeof(KXErrorHandler *)));
    }
    handlers[ pos++ ] = this;
}

bool KXErrorHandler::error(bool sync) const
{
    if (sync) {
        XSync(d->display, False);
    }
    return d->was_error;
}

XErrorEvent KXErrorHandler::errorEvent() const
{
    return d->error_event;
}

int KXErrorHandler::handler_wrapper(Display *dpy, XErrorEvent *e)
{
    --pos;
    int ret = handlers[ pos ]->handle(dpy, e);
    ++pos;
    return ret;
}

int KXErrorHandler::handle(Display *dpy, XErrorEvent *e)
{
    if (dpy == d->display
            // e->serial >= d->first_request , compare like X timestamps to handle wrapping
            && NET::timestampCompare(e->serial, d->first_request) >= 0) {
        // it's for us
        //qDebug( "Handling: %p", static_cast< void* >( this ));
        bool error = false;
        if (user_handler1 != nullptr) {
            if (user_handler1(e->request_code, e->error_code, e->resourceid)) {
                error = true;
            }
        } else if (user_handler2 != nullptr) {
            if (user_handler2(dpy, e) != 0) {
                error = true;
            }
        } else { // no handler set, simply set that there was an error
            error = true;
        }
        if (error && !d->was_error) {
            // only remember the first
            d->was_error = true;
            d->error_event = *e;
        }
        return 0;
    }
    //qDebug( "Going deeper: %p", static_cast< void* >( this ));
    return old_handler(dpy, e);
}

QByteArray KXErrorHandler::errorMessage(const XErrorEvent &event, Display *dpy)
{
    // "Error: <error> (<value>), Request: <request>(<value>), Resource: <value>"
    QByteArray ret;
    char tmp[ 256 ];
    char num[ 256 ];
#if 0 // see below
    if (event.request_code < 128)  // core request
#endif
    {
        XGetErrorText(dpy, event.error_code, tmp, 255);
        if (char *paren = strchr(tmp, '(')) { // the explanation in parentheses just makes
            *paren = '\0';    // it more verbose and is not really useful
        }
        // the various casts are to get overloads non-ambiguous :-/
/*
        ret = QByteArray("error: ") + (const char *)tmp + '[' + QByteArray::number(event.error_code) + ']';
        sprintf(num, "%d", event.request_code);
        XGetErrorDatabaseText(dpy, "XRequest", num, "<unknown>", tmp, 256);
        ret += QByteArray(", request: ") + (const char *)tmp + '[' + QByteArray::number(event.request_code) + ']';
        if (event.resourceid != 0) {
            ret += QByteArray(", resource: 0x") + QByteArray::number((qlonglong)event.resourceid, 16);
        }
*/
    }
#if 0
    else { // extensions
        // XGetErrorText() currently has a bug that makes it fail to find text
        // for some errors (when error==error_base), also XGetErrorDatabaseText()
        // requires the right extension name, so it is needed to get info about
        // all extensions. However that is almost impossible:
        // - Xlib itself has it, but in internal data.
        // - Opening another X connection now can cause deadlock with server grabs.
        // - Fetching it at startup means a bunch of roundtrips.
        // So if this becomes more useful in the future, do the roundtrips at startup,
        // or fetch it in kded and export as an env.var or something.
        Display *dpy2 = XOpenDisplay(XDisplayString(dpy));
        int nextensions;
        char **extensions = XListExtensions(dpy2, &nextensions);
        int *majors = NULL;
        int *error_bases = NULL;
        if (extensions == NULL) {
            nextensions = 0;
        } else {
            majors = new int[ nextensions ];
            error_bases = new int[ nextensions ];
            for (int i = 0;
                    i < nextensions;
                    ++i) {
                int dummy;
                if (!XQueryExtension(dpy2, extensions[ i ], &majors[ i ], &dummy, &error_bases[ i ])) {
                    majors[ i ] = 0;
                    error_bases[ i ] = 0;
                }
            }
        }
        XGetErrorText(dpy, event.error_code, tmp, 255);
        int index = -1;
        int base = 0;
        for (int i = 0;
                i < nextensions;
                ++i)
            if (error_bases[ i ] != 0
                    && event.error_code >= error_bases[ i ] && (index == -1 || error_bases[ i ] > base)) {
                index = i;
                base = error_bases[ i ];
            }
        if (tmp == QString::number(event.error_code)) { // XGetErrorText() failed,
            // or it has a bug that causes not finding all errors, check ourselves
            if (index != -1) {
                qsnprintf(num, 255, "%s.%d", extensions[ index ], event.error_code - base);
                XGetErrorDatabaseText(dpy, "XProtoError", num, "<unknown>", tmp, 255);
            } else {
                strcpy(tmp, "<unknown>");
            }
        }
        if (char *paren = strchr(tmp, '(')) {
            *paren = '\0';
        }
        if (index != -1)
            ret = QByteArray("error: ") + (const char *)tmp + '[' + (const char *)extensions[ index ]
                  + '+' + QByteArray::number(event.error_code - base) + ']';
        else {
            ret = QByteArray("error: ") + (const char *)tmp + '[' + QByteArray::number(event.error_code) + ']';
        }
        tmp[ 0 ] = '\0';
        for (int i = 0;
                i < nextensions;
                ++i)
            if (majors[ i ] == event.request_code) {
                qsnprintf(num, 255, "%s.%d", extensions[ i ], event.minor_code);
                XGetErrorDatabaseText(dpy, "XRequest", num, "<unknown>", tmp, 255);
                ret += QByteArray(", request: ") + (const char *)tmp + '[' + (const char *)extensions[ i ] + '+'
                       + QByteArray::number(event.minor_code) + ']';
            }
        if (tmp[ 0 ] == '\0')  // not found???
            ret += QByteArray(", request <unknown> [") + QByteArray::number(event.request_code) + ':'
                   + QByteArray::number(event.minor_code) + ']';
        if (event.resourceid != 0) {
            ret += QByteArray(", resource: 0x") + QByteArray::number((qlonglong)event.resourceid, 16);
        }
        if (extensions != NULL) {
            XFreeExtensionList(extensions);
        }
        delete[] majors;
        delete[] error_bases;
        XCloseDisplay(dpy2);
    }
#endif
    return ret;
}


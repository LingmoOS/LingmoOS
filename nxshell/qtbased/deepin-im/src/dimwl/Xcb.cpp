// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Xcb.h"

#include <wayland-server-core.h>

#include <iomanip>
#include <stdexcept>

#include <limits.h>
#include <string.h>

Xcb::Xcb(wl_event_loop *loop)
    : xconn_(xcb_connect(nullptr, &defaultScreenNbr_))
{
    if (int err = xcb_connection_has_error(xconn_.get())) {
        return;
    }

    setConnection(xconn_.get());

    setup_ = xcb_get_setup(xconn_.get());
    screen_ = screenOfDisplay(defaultScreenNbr_);

    xcbFd_ = xcb_get_file_descriptor(xconn_.get());
    uint32_t events = WL_EVENT_READABLE | WL_EVENT_ERROR | WL_EVENT_HANGUP;
    wl_event_loop_add_fd(
        loop,
        xcbFd_,
        events,
        [](int fd, uint32_t mask, void *data) -> int {
            auto *p = static_cast<Xcb *>(data);
            return p->onXCBEvent(fd, mask);
        },
        this);
}

Xcb::~Xcb() { }

xcb_screen_t *Xcb::screenOfDisplay(int screen)
{
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup_);
    for (; iter.rem; --screen, xcb_screen_next(&iter)) {
        if (screen == 0) {
            return iter.data;
        }
    }

    return nullptr;
}

std::string Xcb::windowToString(xcb_window_t window)
{
    if (window == 0) {
        return {};
    }

    std::stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(window) * 2) << std::hex << window;
    return stream.str();
}

xcb_window_t Xcb::stringToWindow(const std::string &string)
{
    return std::stoul(string, nullptr, 16);
}

int Xcb::onXCBEvent(int fd, uint32_t mask)
{
    if ((mask & WL_EVENT_HANGUP) || (mask & WL_EVENT_ERROR)) {
        if (mask & WL_EVENT_ERROR) {
            // wlr_log(WLR_ERROR, "Failed to read from X11 server");
        }
        // wl_display_terminate(x11->wl_display);
        return 0;
    }

    std::unique_ptr<xcb_generic_event_t> event;
    while (event.reset(xcb_poll_for_event(xconn_.get())), event) {
        xcbEvent(event);
    }

    int ret = xcb_connection_has_error(xconn_.get());
    if (ret != 0) {
        // wlr_log(WLR_ERROR, "X11 connection error (%d)", ret);
        // wl_display_terminate(x11->wl_display);
    }

    return 0;
}

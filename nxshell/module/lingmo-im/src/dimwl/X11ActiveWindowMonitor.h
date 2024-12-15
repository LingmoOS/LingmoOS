// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef X11ACTIVEWINDOWMONITOR_H
#define X11ACTIVEWINDOWMONITOR_H

#include "Xcb.h"

#include <wayland-server-core.h>

class X11ActiveWindowMonitor : public Xcb
{
public:
    explicit X11ActiveWindowMonitor(wl_event_loop *loop);
    ~X11ActiveWindowMonitor() override;

    xcb_window_t activeWindow();
    pid_t windowPid(xcb_window_t window);
    std::tuple<uint16_t, uint16_t> windowPosition(xcb_window_t window);

protected:
    void xcbEvent(const std::unique_ptr<xcb_generic_event_t> &event) override;

public:
    struct
    {
        struct wl_signal activeWindow;
    } events;

private:
    const std::string atomActiveWindow_;
    const std::string atomWmPid_;

    xcb_window_t currentActiveWindow_;
};

#endif // !X11ACTIVEWINDOWMONITOR_H

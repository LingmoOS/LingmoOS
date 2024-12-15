// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef XCB_H
#define XCB_H

#include "XcbHelper.h"
#include "common/common.h"

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

struct wl_event_loop;

class Xcb : public XcbHelper
{
public:
    Xcb(wl_event_loop *loop);
    virtual ~Xcb();

    static std::string windowToString(xcb_window_t window);
    static xcb_window_t stringToWindow(const std::string &string);

protected:
    std::unique_ptr<xcb_connection_t, Deleter<xcb_disconnect>> xconn_;
    int defaultScreenNbr_;

    xcb_screen_t *screen() { return screen_; }

    virtual void xcbEvent(const std::unique_ptr<xcb_generic_event_t> &event) = 0;

private:
    xcb_screen_t *screenOfDisplay(int screen);
    int onXCBEvent(int fd, uint32_t mask);

private:
    int xcbFd_;
    const xcb_setup_t *setup_;
    xcb_screen_t *screen_;
    std::unordered_map<std::string, xcb_atom_t> atoms_;
};

#endif // !XCB_H

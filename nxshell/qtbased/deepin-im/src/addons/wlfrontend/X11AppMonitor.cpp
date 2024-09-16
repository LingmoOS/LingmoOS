// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "X11AppMonitor.h"

#include <experimental/unordered_map>

#include <QDebug>

using namespace org::deepin::dim;

X11AppMonitor::X11AppMonitor()
    : activeWindow_("_NET_ACTIVE_WINDOW")
    , netClientList_("_NET_CLIENT_LIST")
    , wmPid_("_NET_WM_PID")
    , window_(0)
{
    uint32_t values[] = { XCB_EVENT_MASK_PROPERTY_CHANGE };
    xcb_change_window_attributes(xconn_.get(), screen()->root, XCB_CW_EVENT_MASK, values);

    xcb_flush(xconn_.get());

    init();
}

X11AppMonitor::~X11AppMonitor() = default;

void X11AppMonitor::xcbEvent(const std::unique_ptr<xcb_generic_event_t> &event)
{
    auto responseType = XCB_EVENT_RESPONSE_TYPE(event);
    if (responseType != XCB_PROPERTY_NOTIFY) {
        return;
    }

    auto *e = reinterpret_cast<xcb_property_notify_event_t *>(event.get());
    if (e->atom == getAtom(activeWindow_)) {
        activeWindowChanged();
        return;
    }

    if (e->atom == getAtom(netClientList_)) {
        clientListChanged();
        return;
    }
}

void X11AppMonitor::init()
{
    auto data = getProperty(screen()->root, netClientList_);
    for (int i = 0; i < data.size(); i += sizeof(xcb_window_t)) {
        xcb_window_t window = *reinterpret_cast<xcb_window_t *>(data.data() + i);
        pid_t pid = getWindowPid(window);

        apps_.emplace(windowToString(window), std::to_string(pid));
    }
}

pid_t X11AppMonitor::getWindowPid(xcb_window_t window)
{
    auto data1 = getProperty(window, wmPid_, sizeof(uint32_t));
    if (data1.size() == 0) {
        qWarning() << "failed to get pid";
        return 0;
    }

    return *reinterpret_cast<uint32_t *>(data1.data());
}

std::tuple<int32_t, int32_t> X11AppMonitor::getWindowPosition(xcb_window_t window)
{
    if (window == 0) {
        return { 0, 0 };
    }

    auto geom = XCB_REPLY(xcb_get_geometry, xconn_.get(), window);
    if (!geom) {
        return { 0, 0 };
    }
    auto offset = XCB_REPLY(xcb_translate_coordinates,
                            xconn_.get(),
                            window,
                            screen()->root,
                            geom->x,
                            geom->y);
    return { offset->dst_x, offset->dst_y };
}

void X11AppMonitor::activeWindowChanged()
{
    auto data = getProperty(screen()->root, activeWindow_, sizeof(xcb_window_t));
    if (data.size() == 0) {
        qWarning() << "failed to get active window id";
        return;
    }

    xcb_window_t window = *reinterpret_cast<xcb_window_t *>(data.data());
    if (window == 0 || window == window_) {
        return;
    }

    window_ = window;
    focus_ = windowToString(window);

    emit appUpdated(apps_, focus_);
}

void X11AppMonitor::clientListChanged()
{
    auto data = getProperty(screen()->root, netClientList_);
    if (data.size() == 0) {
        return;
    }

    auto cnt = data.size() / sizeof(xcb_window_t);
    auto *windowIds = reinterpret_cast<xcb_window_t *>(data.data());

    std::experimental::erase_if(apps_, [cnt, &windowIds](const auto &item) {
        const auto &[windowId, _] = item;

        for (uint32_t i = 0; i < cnt; i++) {
            xcb_window_t windowId1 = windowIds[i];
            if (stringToWindow(windowId) == windowId1) {
                return false;
            }
        }

        return true;
    });

    for (uint32_t i = 0; i < cnt; i++) {
        xcb_window_t windowId = windowIds[i];

        auto windowIdStr = windowToString(windowId);
        auto iter = apps_.find(windowIdStr);
        if (iter == apps_.end()) {
            pid_t pid = getWindowPid(windowId);
            apps_.emplace(windowIdStr, std::to_string(pid));
        }
    }

    emit appUpdated(apps_, focus_);
}

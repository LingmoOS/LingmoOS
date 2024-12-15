// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTENER_H
#define LISTENER_H

#include <wayland-server-core.h>

template<auto F>
class Listener;

template<typename C, void (C::*F)(void *data)>
class Listener<F>
{
public:
    Listener(C *parent)
        : parent_(parent)
    {
        listener_.notify = callback;
    }

    ~Listener() { wl_list_remove(&listener_.link); }

    static void callback(struct wl_listener *listener, void *data)
    {
        Listener *p = wl_container_of(listener, p, listener_);
        (p->parent_->*F)(data);
    }

    operator wl_listener *() { return &listener_; }

private:
    C *parent_;
    wl_listener listener_;
};

#endif // !LISTENER_H

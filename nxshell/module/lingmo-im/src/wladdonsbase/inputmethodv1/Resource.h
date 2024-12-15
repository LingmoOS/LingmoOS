// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOURCE_H
#define RESOURCE_H

#include <wayland-server-core.h>
#include "../Global.h"

#include <functional>
#include <memory>

WL_ADDONS_BASE_BEGIN_NAMESPACE

class Resource : public std::enable_shared_from_this<Resource>
{
public:
    Resource(struct wl_client *client,
             const struct wl_interface *interface,
             int version,
             uint32_t id);
    ~Resource() = default;

    struct wl_resource *handle;

    void *object() { return object_; }

    void setImplementation(const void *implementation,
                           void *object,
                           wl_resource_destroy_func_t onDestroy);
    struct wl_client *client();
    void destroy();

private:
    void *object_;
};

template<auto F>
struct ResourceDestroyWrapper;

template<typename C, void (C::*F)(Resource *)>
struct ResourceDestroyWrapper<F>
{
    static void func(struct wl_resource *resource)
    {
        auto *r = static_cast<Resource *>(wl_resource_get_user_data(resource));
        auto *p = static_cast<C *>(r->object());
        (p->*F)(r);
    }
};

template<auto F>
struct ResourceCallbackWrapper;

template<typename C, typename R, typename... Args, R (C::*F)(Resource *, Args...)>
struct ResourceCallbackWrapper<F>
{
    static R func(wl_client *client, wl_resource *resource, Args... args)
    {
        auto *r = static_cast<Resource *>(wl_resource_get_user_data(resource));
        auto *p = static_cast<C *>(r->object());
        (p->*F)(r, args...);
    }
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !RESOURCE_H

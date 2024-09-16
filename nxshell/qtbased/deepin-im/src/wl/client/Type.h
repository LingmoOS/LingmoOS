// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_TYPE_H
#define WL_CLIENT_TYPE_H

#include "wl/Type.h"

#include <wayland-client-protocol.h>

#include <string>

#include <stdint.h>

namespace wl {
namespace client {

template<typename T>
class Type : public wl::Type<T>
{
public:
    Type(T *val)
        : val_(val)
    {
    }

    ~Type() = default;

    T *get() { return val_; }

private:
    T *val_;
};

template<auto F>
struct ListenerCallbackWrapper;

template<typename C, typename R, typename... Args, R (C::*F)(Args...)>
struct ListenerCallbackWrapper<F>
{
    template<typename T>
    static R func(void *userdata, T *, Args... args)
    {
        auto *p = static_cast<C *>(userdata);
        return (p->*F)(args...);
    }
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_TYPE_H

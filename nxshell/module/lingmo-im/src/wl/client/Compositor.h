// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_COMPOSITOR_H
#define WL_CLIENT_COMPOSITOR_H

#include "Type.h"

namespace wl::client {

class Compositor : public Type<wl_compositor>
{
public:
    Compositor(wl_compositor *val);
    ~Compositor();

    wl_surface *create_surface();
};

} // namespace wl::client

#endif // !WL_CLIENT_COMPOSITOR_H

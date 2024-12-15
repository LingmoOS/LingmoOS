// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_SURFACE_H
#define WL_CLIENT_SURFACE_H

#include "Type.h"

namespace wl::client {

class Surface : public Type<wl_surface>
{
public:
    Surface(wl_surface *val);
    virtual ~Surface();
};

} // namespace wl::client

#endif // !WL_CLIENT_SURFACE_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Compositor.h"

using namespace wl::client;

Compositor::Compositor(wl_compositor *val)
    : Type(val)
{
}

Compositor::~Compositor() = default;

wl_surface *Compositor::create_surface()
{
    return wl_compositor_create_surface(get());
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Surface.h"

using namespace wl::client;

Surface::Surface(wl_surface *val)
    : Type(val)
{
}

Surface::~Surface() = default;

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ConnectionRaw.h"

using namespace wl::client;

ConnectionRaw::ConnectionRaw(struct wl_display *display)
    : display_(display)
{
    init();
}

ConnectionRaw::~ConnectionRaw() { }

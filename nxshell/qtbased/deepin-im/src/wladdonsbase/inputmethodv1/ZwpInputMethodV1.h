// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_ADDONS_BASE_ZWPINPUTMETHODV1_H
#define WL_ADDONS_BASE_ZWPINPUTMETHODV1_H

#include "Type.h"
#include "wayland-input-method-unstable-v1-server-protocol.h"

WL_ADDONS_BASE_BEGIN_NAMESPACE

class ZwpInputMethodV1 : public Type<ZwpInputMethodV1, zwp_input_method_v1>
{
    friend class Type;

public:
    ZwpInputMethodV1();
    ~ZwpInputMethodV1() override;

    void send_activate(struct wl_resource *resource, struct wl_resource *id);
    void send_deactivate(struct wl_resource *resource, struct wl_resource *context);

private:
    static const struct ::wl_interface impl;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !WL_ADDONS_BASE_ZWPINPUTMETHODV1_H

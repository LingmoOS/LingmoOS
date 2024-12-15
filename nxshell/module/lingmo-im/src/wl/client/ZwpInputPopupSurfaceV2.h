// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_ZWPINPUTPOPUPSURFACEV2_H
#define WL_CLIENT_ZWPINPUTPOPUPSURFACEV2_H

#include "Type.h"

#include <memory>

namespace wl {
namespace client {

class ZwpInputPopupSurfaceV2 : public Type<zwp_input_popup_surface_v2>
{
public:
    ZwpInputPopupSurfaceV2(zwp_input_popup_surface_v2 *val);
    virtual ~ZwpInputPopupSurfaceV2();

protected:
    virtual void zwp_input_popup_surface_v2_text_input_rectangle(int32_t x,
                                                                 int32_t y,
                                                                 int32_t width,
                                                                 int32_t height) = 0;

private:
    static const zwp_input_popup_surface_v2_listener listener_;
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_ZWPINPUTPOPUPSURFACEV2_H

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTPOPUPSURFACEV2_H
#define INPUTPOPUPSURFACEV2_H

#include "wl/client/ZwpInputPopupSurfaceV2.h"

#include <QObject>
#include <QString>

namespace org {
namespace deepin {
namespace dim {

class InputPopupSurfaceV2 : public wl::client::ZwpInputPopupSurfaceV2
{
public:
    explicit InputPopupSurfaceV2(zwp_input_popup_surface_v2 *val);
    virtual ~InputPopupSurfaceV2();

protected:
    void zwp_input_popup_surface_v2_text_input_rectangle(int32_t x,
                                                         int32_t y,
                                                         int32_t width,
                                                         int32_t height) override;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !INPUTPOPUPSURFACEV2_H

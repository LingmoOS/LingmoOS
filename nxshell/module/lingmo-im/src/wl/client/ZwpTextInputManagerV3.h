// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_ZWPTEXTINPUTMANAGERV3_H
#define WL_CLIENT_ZWPTEXTINPUTMANAGERV3_H

#include "Type.h"
#include "wayland-text-input-unstable-v3-client-protocol.h"

#include <memory>

namespace wl {
namespace client {

class ZwpTextInputV3;
class Seat;

class ZwpTextInputManagerV3 : public Type<zwp_text_input_manager_v3>
{
public:
    ZwpTextInputManagerV3(zwp_text_input_manager_v3 *val);
    ~ZwpTextInputManagerV3();

    struct zwp_text_input_v3 *get_text_input(const std::shared_ptr<Seat> &seat);

private:
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_ZWPTEXTINPUTMANAGERV3_H

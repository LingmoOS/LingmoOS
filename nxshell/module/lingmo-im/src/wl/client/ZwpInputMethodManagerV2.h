// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_ZWPINPUTMETHODMANAGERV2_H
#define WL_CLIENT_ZWPINPUTMETHODMANAGERV2_H

#include "Type.h"

#include <memory>

namespace wl {
namespace client {

class Seat;
class ZwpInputMethodV2;

class ZwpInputMethodManagerV2 : public Type<zwp_input_method_manager_v2>
{
public:
    ZwpInputMethodManagerV2(zwp_input_method_manager_v2 *val);
    ~ZwpInputMethodManagerV2();

    zwp_input_method_v2 *get_input_method(const std::shared_ptr<Seat> &seat);
};

} // namespace client
}; // namespace wl

#endif // !WL_CLIENT_ZWPINPUTMETHODMANAGERV2_H

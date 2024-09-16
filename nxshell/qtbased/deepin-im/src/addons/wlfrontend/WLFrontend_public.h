// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WLFRONTEND_PUBLIC_H
#define WLFRONTEND_PUBLIC_H

namespace wl::client {
class ZwpInputMethodV2;
class ConnectionBase;
} // namespace wl::client

namespace org::deepin::dim {

class InputContext;
class Addon;

namespace wlfrontend {

wl::client::ZwpInputMethodV2 *getInputMethodV2(InputContext *ic);

wl::client::ConnectionBase *getWl(Addon *addon);

} // namespace wlfrontend

} // namespace org::deepin::dim

#endif // !WLFRONTEND_PUBLIC_H

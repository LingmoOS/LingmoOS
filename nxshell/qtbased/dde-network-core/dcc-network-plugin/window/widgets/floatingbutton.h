// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FLOATINGBUTTON_H
#define FLOATINGBUTTON_H
#include <DFloatingButton>

namespace dde {
namespace network {
class FloatingButton : public DTK_WIDGET_NAMESPACE::DFloatingButton
{
public:
    explicit FloatingButton()
        : DFloatingButton(nullptr) {}
};
}
}

#endif // FLOATINGBUTTON_H

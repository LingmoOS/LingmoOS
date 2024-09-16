// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodAddon.h"

using namespace org::deepin::dim;

InputMethodAddon::InputMethodAddon(Dim *dim, const std::string &key, const QString &iconName)
    : Addon(dim, key)
    , iconName_(iconName)
{
}

InputMethodAddon::~InputMethodAddon() { }

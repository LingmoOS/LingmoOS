// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRONTENDADDON_H
#define FRONTENDADDON_H

#include "Addon.h"

namespace org {
namespace deepin {
namespace dim {

class FrontendAddon : public Addon
{
    Q_OBJECT

public:
    explicit FrontendAddon(Dim *dim, const std::string &key);
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !FRONTENDADDON_H

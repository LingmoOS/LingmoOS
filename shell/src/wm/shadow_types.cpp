// Copyright 2015 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/shadow_types.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wm/shadow_types.h"

namespace Lingmo::WM {

const char *toString(ShadowElevation elevation)
{
    switch (elevation) {
    case ShadowElevation::None:
        return "none";
    case ShadowElevation::Dock:
        return "dock";
    case ShadowElevation::Popup:
        return "popup";
    case ShadowElevation::MenuOrTooltip:
        return "menu";
    case ShadowElevation::Inactive:
        return "inactive";
    case ShadowElevation::Active:
        return "active";
    }
    return "";
}

} // namespace Lingmo::WM

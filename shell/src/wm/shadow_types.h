// Copyright 2015 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/shadow_types.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_WM_SHADOW_TYPES_H_
#define LINGMO_WM_SHADOW_TYPES_H_

namespace Lingmo::WM {

// How much shadow a window casts. Upstream these are dip elevations carried in
// a window property (kNone, kDefault, kInactiveWindow, kActiveWindow,
// kMenuOrTooltip) and turned into blur/offset by ui::Shadow. Lingmo keeps
// FishUI's own two-lobe style table instead, so the enum names the role and
// decoration::Metrics owns the numbers.
enum class ShadowElevation {
    None,
    Popup,
    Dock,
    MenuOrTooltip,
    Inactive,
    Active,
};

const char *toString(ShadowElevation elevation);

} // namespace Lingmo::WM

#endif // LINGMO_WM_SHADOW_TYPES_H_
